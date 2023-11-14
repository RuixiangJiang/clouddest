// 引入ns-3的核心头文件
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"

// 启用ns-3的命名空间
using namespace ns3;

// 启用日志组件，可以根据需要启用或禁用
NS_LOG_COMPONENT_DEFINE("TcpExample");

int main(int argc, char *argv[]) {
    // 日志级别可以根据需要进行调整
    LogComponentEnable("TcpExample", LOG_LEVEL_INFO);

    // 创建两个节点
    NodeContainer nodes;
    nodes.Create(2);

    // 设置点对点连接属性
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    // 安装网络设备到节点
    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);

    // 安装TCP/IP协议栈到节点
    InternetStackHelper stack;
    stack.Install(nodes);

    // 分配IP地址
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // 创建TCP服务器应用程序
    uint16_t serverPort = 8080;
    Address serverAddress(InetSocketAddress(interfaces.GetAddress(1), serverPort));
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", serverAddress);
    ApplicationContainer serverApps = packetSinkHelper.Install(nodes.Get(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    // 创建TCP客户端应用程序
    OnOffHelper onOffHelper("ns3::TcpSocketFactory", Address());
    onOffHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onOffHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    onOffHelper.SetAttribute("PacketSize", UintegerValue(1024));
    onOffHelper.SetAttribute("DataRate", StringValue("1Mbps"));
    onOffHelper.SetAttribute("Remote", AddressValue(serverAddress));
    ApplicationContainer clientApps = onOffHelper.Install(nodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    // 启动流量监控
    FlowMonitorHelper flowMonitor;
    Ptr<FlowMonitor> monitor = flowMonitor.InstallAll();

    // 启动仿真
    Simulator::Stop(Seconds(11.0));
    Simulator::Run();

    // 输出流量监控结果
    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowMonitor.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin(); i != stats.end(); ++i) {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
        std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
        std::cout << "  Tx Bytes: " << i->second.txBytes << "\n";
        std::cout << "  Rx Bytes: " << i->second.rxBytes << "\n";
        std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) / 1024 / 1024  << " Mbps\n";
    }

    // 结束仿真
    Simulator::Destroy();
    return 0;
}