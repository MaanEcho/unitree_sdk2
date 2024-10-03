#include <unitree/robot/channel/channel_publisher.hpp>
#include <unitree/common/time/time_tool.hpp>
#include "HelloWorldData.hpp"

#define TOPIC "TopicHelloWorld"

using namespace unitree::robot;
using namespace unitree::common;

int main()
{
    ChannelFactory::Instance()->Init(0);
    // Instance()：获取单例静态指针
    // Init(0)：指定 Domain Id、网卡名、是否使用共享内存三个初始化参数，对 ChannelFactory 进行初始化。
    // 0：Domain Id；networkInterface：指定网卡名，默认为空；tenableSharedMemory：是否使用共享内存，默认为 false。

    // unitree::robot::ChannelPublisher 类实现了指定类型的消息发布功能
    ChannelPublisher<HelloWorldData::Msg> publisher(TOPIC);

    publisher.InitChannel();
    // 初始化 Channel，准备用于发送消息。

    while (true)
    {
        HelloWorldData::Msg msg(unitree::common::GetCurrentTimeMillisecond(), "HelloWorld.");
        publisher.Write(msg);
        // 发送消息
        sleep(1);
    }

    return 0;
}
