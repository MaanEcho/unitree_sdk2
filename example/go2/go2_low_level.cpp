#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <unitree/robot/channel/channel_publisher.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>
#include <unitree/idl/go2/LowState_.hpp>
#include <unitree/idl/go2/LowCmd_.hpp>
#include <unitree/common/time/time_tool.hpp>
#include <unitree/common/thread/thread.hpp>

using namespace unitree::common;
using namespace unitree::robot;

#define TOPIC_LOWCMD "rt/lowcmd"
#define TOPIC_LOWSTATE "rt/lowstate"

constexpr double PosStopF = (2.146E+9f);
constexpr double VelStopF = (16000.0f);

class Custom // 阅读完成
{
public:
    explicit Custom()
    {
    }

    ~Custom()
    {
    }

    void Init();

private:
    void InitLowCmd();
    void LowStateMessageHandler(const void *messages);
    void LowCmdWrite();

private:
    float qInit[3] = {0};
    float qDes[3] = {0};
    float sin_mid_q[3] = {0.0, 1.2, -2.0};
    float Kp[3] = {0};
    float Kd[3] = {0};
    double time_consume = 0;
    int rate_count = 0;
    int sin_count = 0;
    int motiontime = 0; // 底层控制指令的下发次数
    float dt = 0.002;   // 0.001~0.01 控制步长

    unitree_go::msg::dds_::LowCmd_ low_cmd{};     // default init
    unitree_go::msg::dds_::LowState_ low_state{}; // default init

    /*publisher*/
    ChannelPublisherPtr<unitree_go::msg::dds_::LowCmd_> lowcmd_publisher;
    /*subscriber*/
    ChannelSubscriberPtr<unitree_go::msg::dds_::LowState_> lowstate_subscriber;

    /*LowCmd write thread*/
    ThreadPtr lowCmdWriteThreadPtr;
};

// Unitree 提供的电机校验函数
uint32_t crc32_core(uint32_t *ptr, uint32_t len) // 阅读完成
{
    unsigned int xbit = 0;
    unsigned int data = 0;
    unsigned int CRC32 = 0xFFFFFFFF;
    const unsigned int dwPolynomial = 0x04c11db7;

    for (unsigned int i = 0; i < len; i++)
    {
        xbit = 1 << 31;
        data = ptr[i];
        for (unsigned int bits = 0; bits < 32; bits++)
        {
            if (CRC32 & 0x80000000)
            {
                CRC32 <<= 1;
                CRC32 ^= dwPolynomial;
            }
            else
            {
                CRC32 <<= 1;
            }

            if (data & xbit)
                CRC32 ^= dwPolynomial;
            xbit >>= 1;
        }
    }

    return CRC32;
}

// Init() 该函数用于初始化设置 publisher、subscriber、发布线程等。
void Custom::Init() // 阅读完成
{
    InitLowCmd();

    /*create publisher*/
    lowcmd_publisher.reset(new ChannelPublisher<unitree_go::msg::dds_::LowCmd_>(TOPIC_LOWCMD));
    lowcmd_publisher->InitChannel();

    /*create subscriber*/
    lowstate_subscriber.reset(new ChannelSubscriber<unitree_go::msg::dds_::LowState_>(TOPIC_LOWSTATE));
    lowstate_subscriber->InitChannel(std::bind(&Custom::LowStateMessageHandler, this, std::placeholders::_1), 1);

    /*loop publishing thread*/
    lowCmdWriteThreadPtr = CreateRecurrentThreadEx("writebasiccmd", UT_CPU_ID_NONE, 2000, &Custom::LowCmdWrite, this);
}

// InitLowCmd() 该函数用于初始化设置 LowCmd 类型的 low_cmd 结构体。该函数放置于 Custom 类的构造函数运行一次即可。
void Custom::InitLowCmd() // 阅读完成
{
    // LowCmd 类型中的 head 成员表示帧头，此帧头将用于 CRC 校验。head、levelFlag、gpio 等按例程所示设置为默认值即可。
    low_cmd.head()[0] = 0xFE;
    low_cmd.head()[1] = 0xEF;
    low_cmd.level_flag() = 0xFF;
    low_cmd.gpio() = 0;

    // LowCmd 类型中有 20 个 motor_cmd 成员，每一个成员的命令用于控制 Go2 机器人上相对应的一个电机，但 Go2 机器人上只有 12 个电机，故仅有前 12 个有效，剩余的 8 个起保留作用。
    for (int i = 0; i < 20; i++)
    {
        low_cmd.motor_cmd()[i].mode() = (0x01); // motor switch to servo (PMSM) mode
        // 此行命令中将 motor_cmd 成员的 mode 变量设置为 0x01，0x01 表示将电机设置为伺服模式。如果用户在调试过程中发现无法控制 Go2 机器人的关节电机，请检查变量的值是否为0x01。
        low_cmd.motor_cmd()[i].q() = (PosStopF);
        low_cmd.motor_cmd()[i].dq() = (VelStopF);
        low_cmd.motor_cmd()[i].kp() = (0);
        low_cmd.motor_cmd()[i].kd() = (0);
        low_cmd.motor_cmd()[i].tau() = (0);
    }
}

// LowStateMessageHandler() 该函数用于处理 Go2 机器人上电机状态信息的回调函数。该函数的作用是将 Go2 机器人上电机状态信息保存至 low_state 结构体中。
void Custom::LowStateMessageHandler(const void *message) // 阅读完成
{
    low_state = *(unitree_go::msg::dds_::LowState_ *)message;
}

// 此函数用于计算两个关节角度的线性插值。
double jointLinearInterpolation(double initPos, double targetPos, double rate) // 阅读完成
{
    double p;
    rate = std::min(std::max(rate, 0.0), 1.0);
    p = initPos * (1 - rate) + targetPos * rate;
    return p;
}

// 此函数是发送底层控制指令的回调函数，DDS 会以一定频率触发该回调函数。此回调函数的前大部分代码表示计算右前小腿摆动控制命令的用户逻辑，后小部分代码将控制命令发送至 Go2 机器人。
void Custom::LowCmdWrite() // 阅读完成
{
    motiontime++;

    if (motiontime >= 0)
    {
        // first, get record initial position
        if (motiontime >= 0 && motiontime < 20)
        {
            qInit[0] = low_state.motor_state()[0].q();
            qInit[1] = low_state.motor_state()[1].q();
            qInit[2] = low_state.motor_state()[2].q();
        }
        // second, move to the origin point of a sine movement with Kp Kd
        if (motiontime >= 10 && motiontime < 400)
        {
            rate_count++;

            double rate = rate_count / 200.0; // needs count to 200
            Kp[0] = 5.0;
            Kp[1] = 5.0;
            Kp[2] = 5.0;
            Kd[0] = 1.0;
            Kd[1] = 1.0;
            Kd[2] = 1.0;

            qDes[0] = jointLinearInterpolation(qInit[0], sin_mid_q[0], rate);
            qDes[1] = jointLinearInterpolation(qInit[1], sin_mid_q[1], rate);
            qDes[2] = jointLinearInterpolation(qInit[2], sin_mid_q[2], rate);
        }

        double sin_joint1, sin_joint2;
        // last, do sine wave
        float freq_Hz = 1;
        // float freq_Hz = 5;
        float freq_rad = freq_Hz * 2 * M_PI;
        float t = dt * sin_count;

        if (motiontime >= 400)
        {
            sin_count++;
            sin_joint1 = 0.6 * sin(t * freq_rad);
            sin_joint2 = -0.9 * sin(t * freq_rad);
            qDes[0] = sin_mid_q[0];
            qDes[1] = sin_mid_q[1] + sin_joint1;
            qDes[2] = sin_mid_q[2] + sin_joint2;
        }

        low_cmd.motor_cmd()[2].q() = qDes[2];
        low_cmd.motor_cmd()[2].dq() = 0;
        low_cmd.motor_cmd()[2].kp() = Kp[2];
        low_cmd.motor_cmd()[2].kd() = Kd[2];
        low_cmd.motor_cmd()[2].tau() = 0;
    }

    low_cmd.crc() = crc32_core((uint32_t *)&low_cmd, (sizeof(unitree_go::msg::dds_::LowCmd_) >> 2) - 1);
    // 计算 CRC 校验码。
    lowcmd_publisher->Write(low_cmd);
    // 调用 lowcmd_publisher 的 Write() 函数将控制命令发送给 Go2 机器人。
}

// 主函数
int main(int argc, const char **argv) // 阅读完成
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " networkInterface" << std::endl;
        exit(-1);
    }

    ChannelFactory::Instance()->Init(0, argv[1]);

    Custom custom;
    custom.Init();

    while (1)
    {
        sleep(10);
    }

    return 0;
}
