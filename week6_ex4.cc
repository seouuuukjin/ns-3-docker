#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include <fstream>
#include <iostream>
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("w6_4");

static void CwndChange(uint32_t oldVal, uint32_t newVal){
    NS_LOG_INFO(Simulator::Now().GetSeconds() << "\t" << newVal);
}

static void RxDrop(Ptr<const Packet> p){
    NS_LOG_LOGIC("RxDrops at " << Simulator::Now().GetSeconds());
}

int 
main(int argc, char*argv[])
{
    LogComponentEnable("w6_4", LOG_LEVEL_ALL);
                        
    //NS_LOG_INFO("Create Nodes");    
    NodeContainer nodes;
    nodes.Create(2);
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices;
    devices = p2p.Install(nodes);
    //Define Error Model
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
    em->SetAttribute("ErrorRate", DoubleValue(0.00001));
    devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));


    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper addr;
    addr.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = addr.Assign(devices);
                   
    uint16_t sinkPort = 8080;
    Address sinkAddress (InetSocketAddress(interfaces.GetAddress(1), sinkPort));
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sinkPort));
    ApplicationContainer sinkApp = packetSinkHelper.Install(nodes.Get(1));
    sinkApp.Start(Seconds(0.));
    sinkApp.Stop(Seconds(10.));

    devices.Get(1)->TraceConnectWithoutContext("PhyRxDrop", MakeCallback(&RxDrop));

    OnOffHelper onoff("ns3::TcpSocketFactory", sinkAddress);
    onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    onoff.SetAttribute("DataRate", DataRateValue(1000000));
    ApplicationContainer sourceApp = onoff.Install(nodes.Get(0));
    sourceApp.Start(Seconds(1.));
    sourceApp.Stop(Seconds(10.));

    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(nodes.Get(0), TcpSocketFactory::GetTypeId());
    ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&CwndChange));
    nodes.Get(0)->GetApplication(0)->GetObject<OnOffApplication>()->SetSocket(ns3TcpSocket);
    

    //UdpEchoClientHelper echoClient(interfaces.GetAddress(1), 9);
    //echoClient.SetAttribute("MaxPackets", UintegerValue(100));
    //echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    //echoClient.SetAttribute("PacketSize", UintegerValue(1024));
    
    //ApplicationContainer clientApps;
    //clientApps.Add(echoClient.Install(nodes.Get(0)));
    
    //clientApps.Start(Seconds(1.0));
    //clientApps.Stop(Seconds(10.0));
    
    //UdpEchoServerHelper echoServer(9);
    //ApplicationContainer serverApps(echoServer.Install(nodes.Get(1)));
    
    //serverApps.Start(Seconds(0));
    //serverApps.Stop(Seconds(11.0));
    
    Simulator::Run();
    Simulator::Stop(Seconds(10.0));
    Simulator::Destroy();
    return 0;
}

