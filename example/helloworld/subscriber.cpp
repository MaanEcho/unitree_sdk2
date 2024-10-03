#include <unitree/robot/channel/channel_subscriber.hpp>
#include <unitree/common/time/time_tool.hpp>
#include "HelloWorldData.hpp"

#define TOPIC "TopicHelloWorld"

using namespace unitree::robot;
using namespace unitree::common;

void Handler(const void *msg)
{
    const HelloWorldData::Msg *pm = (const HelloWorldData::Msg *)msg;

    std::cout << "userID:" << pm->userID() << ", message:" << pm->message() << std::endl;
}

int main()
{
    ChannelFactory::Instance()->Init(0);
    // Instance()：获取单例静态指针
    // Init(0)：指定 Domain Id、网卡名、是否使用共享内存三个初始化参数，对 ChannelFactory 进行初始化。
    // 0：Domain Id；networkInterface：指定网卡名，默认为空；tenableSharedMemory：是否使用共享内存，默认为 false。

    // unitree::robot::ChannelSubscriber实现了指定类型的消息订阅功能
    ChannelSubscriber<HelloWorldData::Msg> subscriber(TOPIC);
    subscriber.InitChannel(Handler);
    // 初始化 Channel，准备用于接受或处理消息。
    // Handler：消息到达时的回调函数；queuelen：消息缓存队列的长度，默认为 0；如果长度为 0，不启用消息缓存内列。

    sleep(5);
    subscriber.CloseChannel();
    // 关闭 Channel

    std::cout << "reseted. sleep 3" << std::endl;

    sleep(3);
    subscriber.InitChannel();

    sleep(5);
    subscriber.CloseChannel();

    std::cout << "reseted. sleep 3" << std::endl;

    sleep(3);
    subscriber.InitChannel();

    while (true)
    {
        sleep(10);
    }

    return 0;
}
