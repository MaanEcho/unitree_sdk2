#include <mutex>

// #include <go2_idl/WirelessController_.hpp>
#include "unitree/idl/go2/WirelessController_.hpp"
#include "unitree/common/thread/thread.hpp"
#include "unitree/robot/channel/channel_subscriber.hpp"

#include "advanced_gamepad.hpp"

#define TOPIC_JOYSTICK "rt/wirelesscontroller"

using namespace unitree::common;
using namespace unitree::robot;

class GamepadExample
{
public:
    GamepadExample() {}

    // setup dds model
    // 初始化 DDS 模型
    void InitDdsModel(const std::string &networkInterface = "") // 阅读完成
    {
        ChannelFactory::Instance()->Init(0, networkInterface);

        joystick_subscriber.reset(new ChannelSubscriber<unitree_go::msg::dds_::WirelessController_>(TOPIC_JOYSTICK));
        joystick_subscriber->InitChannel(std::bind(&GamepadExample::MessageHandler, this, std::placeholders::_1), 1);
    }

    // set gamepad dead_zone parameter
    // 设置遥控器的死区参数
    void SetGamepadDeadZone(float deadzone) // 阅读完成
    {
        gamepad.dead_zone = deadzone;
    }

    // set gamepad smooth parameter
    // 设置遥控器的平滑参数
    void setGamepadSmooth(float smooth) // 阅读完成
    {
        gamepad.smooth = smooth;
    }

    // callback function to save joystick message
    // 回调函数，用于保存遥控器消息
    void MessageHandler(const void *message) // 阅读完成
    {
        std::lock_guard<std::mutex> lock(joystick_mutex); // 加锁，防止数据竞争
        joystick_msg = *(unitree_go::msg::dds_::WirelessController_ *)message;
    }

    // work thread
    // 工作线程
    void Step() // 阅读完成
    {
        {
            std::lock_guard<std::mutex> lock(joystick_mutex);
            gamepad.Update(joystick_msg);
        }

        // some operations
        if (gamepad.A.on_press)
        {
            press_count += 1;
        }

        // print gamepad state
        std::cout << "lx: " << gamepad.lx << std::endl
                  << "A: pressed: " << gamepad.A.pressed
                  << "; on_press: " << gamepad.A.on_press
                  << "; on_release: " << gamepad.A.on_release
                  << std::endl
                  << "press count: " << press_count
                  << std::endl
                  << "===========================" << std::endl;
    }

    // start the work thread
    // 启动工作线程
    void Start() // 阅读完成
    {
        control_thread_ptr = CreateRecurrentThreadEx("nn_ctrl", UT_CPU_ID_NONE, 40000, &GamepadExample::Step, this);
    }

protected:
    ChannelSubscriberPtr<unitree_go::msg::dds_::WirelessController_> joystick_subscriber;
    unitree_go::msg::dds_::WirelessController_ joystick_msg;

    Gamepad gamepad;

    ThreadPtr control_thread_ptr;

    std::mutex joystick_mutex;

    int press_count = 0;
};

int main() // 阅读完成
{
    // create example object
    GamepadExample example;

    // set gamepad params
    example.setGamepadSmooth(0.2);
    example.SetGamepadDeadZone(0.5);

    // start program
    example.InitDdsModel();
    example.Start();

    while (1)
    {
        usleep(20000);
    }
    return 0;
}