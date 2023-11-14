#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ping-helper.h"

using namespace ns3;

int main(int argc, char *argv[])
{
    // 设置日志组件
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // 创建节点
    NodeContainer nodes;
    nodes.Create(2);

    // 创建点对点链路和设备
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);

    // 安装网络协议栈
    InternetStackHelper stack;
    stack.Install(nodes);

    // 分配IP地址
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // 创建和配置TCP服务器应用
    uint16_t tcpPort = 50000;
    Address tcpServerAddress(InetSocketAddress(interfaces.GetAddress(1), tcpPort));
    PacketSinkHelper tcpSinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), tcpPort));
    ApplicationContainer tcpServerApps = tcpSinkHelper.Install(nodes.Get(1));
    tcpServerApps.Start(Seconds(1.0));
    tcpServerApps.Stop(Seconds(10.0));

    // 创建和配置TCP客户端应用
    OnOffHelper tcpClient("ns3::TcpSocketFactory", Address());
    tcpClient.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    tcpClient.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    tcpClient.SetAttribute("PacketSize", UintegerValue(1024));
    tcpClient.SetAttribute("DataRate", StringValue("1Mbps"));
    tcpClient.SetAttribute("Remote", AddressValue(tcpServerAddress));
    ApplicationContainer tcpClientApps = tcpClient.Install(nodes.Get(0));
    tcpClientApps.Start(Seconds(2.0));
    tcpClientApps.Stop(Seconds(10.0));

    // 创建和配置UDP服务器应用
    uint16_t udpPort = 50001;
    Address udpServerAddress(InetSocketAddress(interfaces.GetAddress(1), udpPort));
    PacketSinkHelper udpSinkHelper("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), udpPort));
    ApplicationContainer udpServerApps = udpSinkHelper.Install(nodes.Get(1));
    udpServerApps.Start(Seconds(1.0));
    udpServerApps.Stop(Seconds(10.0));

    // 创建和配置UDP客户端应用
    OnOffHelper udpClient("ns3::UdpSocketFactory", Address());
    udpClient.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    udpClient.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    udpClient.SetAttribute("PacketSize", UintegerValue(512));
    udpClient.SetAttribute("DataRate", StringValue("1Mbps"));
    udpClient.SetAttribute("Remote", AddressValue(udpServerAddress));
    ApplicationContainer udpClientApps = udpClient.Install(nodes.Get(0));
    udpClientApps.Start(Seconds(2.5));
    udpClientApps.Stop(Seconds(10.0));

    // 创建和配置Ping应用
    PingHelper ping(interfaces.GetAddress(1));
    ping.SetAttribute("VerboseMode", EnumValue(Ping::VerboseMode::VERBOSE));
    ApplicationContainer pingApps = ping.Install(nodes.Get(0));
    pingApps.Start(Seconds(2.0));
    pingApps.Stop(Seconds(10.0));

    // 启用路由
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // 运行模拟
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}