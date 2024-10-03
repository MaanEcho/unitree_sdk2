// 设备状态服务接口例程
// RobotStateClient 是设备状态服务提供的 Client，通过 RobotClient 可以方便地通过 RPC 方式实现对 Go2 内部的服务控制、获取服务状态、设备状态和系统资源使用信息等功能（部分功能接口暂未开放）。
#include <unitree/robot/go2/robot_state/robot_state_client.hpp>
#include <unitree/common/time/time_tool.hpp>

using namespace unitree::common;
using namespace unitree::robot;
using namespace unitree::robot::go2;

int main(int32_t argc, const char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: robot_state_client_example [NetWorkInterface(eth0)] [ServiceName(sport_mode)]" << std::endl;
        exit(0);
    }

    std::string networkInterface = "eth0", serviceName = "sport_mode";

    if (argc > 1)
    {
        networkInterface = argv[1];
    }

    if (argc > 2)
    {
        serviceName = argv[2];
    }

    std::cout << "NetWorkInterface:" << networkInterface << std::endl;
    std::cout << "Switch ServiceName:" << serviceName << std::endl;

    ChannelFactory::Instance()->Init(0, networkInterface);
    // Instance()：获取单例静态指针
    // Init(0, networkInterface)：指定 Domain Id、网卡名、是否使用共享内存三个初始化参数，对 ChannelFactory 进行初始化。
    // 0：Domain Id；networkInterface：指定网卡名；tenableSharedMemory：是否使用共享内存，默认为 false。

    RobotStateClient rsc;
    rsc.SetTimeout(10.0f);
    // 设置 RPC 请求超时时间，单位为秒。
    rsc.Init();
    // 客户端初始化，完成客户端 API 注册等逻辑。

    std::string clientApiVersion = rsc.GetApiVersion();
    // 获取客户端 API 版本
    std::string serverApiVersion = rsc.GetServerApiVersion();
    // 获取服务端 API 版本

    if (clientApiVersion != serverApiVersion)
    {
        std::cout << "client and server api versions are not equal." << std::endl;
    }

    Timer timer;

    int32_t status;
    int32_t ret = rsc.SetReportFreq(3, 30);
    // 设置服务状态上报频率。3：上报时间间隔，单位为秒；30：上报持续时间，单位为秒。
    std::cout << "Call SetReportFreq[3,30] ret:" << ret << ", cost:" << timer.Stop() << " (us)" << std::endl;

    sleep(5);
    timer.Restart();

    ret = rsc.ServiceSwitch(serviceName, 0, status);
    // 服务开关。serviceName：服务名；0：开关，1 开启，0 关闭；status：操作执行后的服务状态。
    std::cout << "Call ServiceSwitch[" << serviceName << ",0] ret:" << ret << ", cost:" << timer.Stop() << " (us)" << std::endl;

    sleep(5);
    timer.Restart();

    ret = rsc.ServiceSwitch(serviceName, 1, status);
    // 服务开关。serviceName：服务名；1：开关，1 开启，0 关闭；status：操作执行后的服务状态。
    std::cout << "Call ServiceSwitch[" << serviceName << ",1] ret:" << ret << ", cost:" << timer.Stop() << " (us)" << std::endl;

    sleep(5);
    timer.Restart();

    std::vector<ServiceState> serviceStateList;
    ret = rsc.ServiceList(serviceStateList);
    std::cout << "Call ServiceList ret:" << ret << ", cost:" << timer.Stop() << " (us)" << std::endl;

    size_t i, count = serviceStateList.size();
    std::cout << "serviceStateList size:" << count << std::endl;

    for (i = 0; i < count; i++)
    {
        const ServiceState &serviceState = serviceStateList[i];
        std::cout << "name:" << serviceState.name << ", status:" << serviceState.status << ", protect:" << serviceState.protect << std::endl;
    }

    ChannelFactory::Instance()->Release();

    return 0;
}
