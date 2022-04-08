#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

#include <iostream>
using namespace ns3;

//NS_LOG_COMPONENT_DEFINE("week7");

int 
main(int argc, char*argv[])
{
    //when <./waf --run "week7_ex1 --howmany=2 --delay=10ms"> command runs, these arguments goes in as written below
    CommandLine cmd;
    std::string datarate = "10";
    std::string delay;
    cmd.AddValue("DataRate", "DataRate removing Mbps", datarate);
    cmd.AddValue("Delay", "Link Delay", delay);
    cmd.Parse(argc, argv);


	//출력하고 싶은 로그들을 허용
	//LogComponentEnable("UdpClientApplication", LOG_LEVEL_INFO);
	//LogComponentEnable("UdpClientApplication", LOG_PREFIX_TIME);
	//LogComponentEnable("UdpClientApplication", LOG_PREFIX_FUNC);

	//LogComponentEnable("UdpServerApplication", LOG_LEVEL_INFO);
	//LogComponentEnable("UdpServerApplication", LOG_PREFIX_TIME);
	//LogComponentEnable("UdpServerApplication", LOG_PREFIX_FUNC);

	//LogComponentEnable("week7", LOG_LEVEL_ALL);
	
	//NS_LOG_INFO("2017314585 Seokjin Hwang");	
	//NS_LOG_INFO("Create Nodes");	

	//노드 컨테이너를 생성해서 노드들을 이 네트워크 topology에서 사용할 노드들을 담아둔다.
	NodeContainer nodes; //make empty bottle for nodes
	nodes.Create(2); //2개 생성
	//NodeContainer nc1 = NodeContainer(nodes.Get(0), nodes.Get(1));
	//NodeContainer nc2 = NodeContainer(nodes.Get(1), nodes.Get(2));

    //week4 added
	//채널의 특성을 설정해준다.
	//PointToPointHelper p2p_1;
	//p2p_1.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
	//p2p_1.SetChannelAttribute("Delay", StringValue("5ms"));

	//PointToPointHelper p2p_2;
	//p2p_2.SetDeviceAttribute("DataRate", StringValue("0.1Mbps"));
	//p2p_2.SetChannelAttribute("Delay", StringValue("10ms"));
        
    //week7 added
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(datarate + "Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue(delay));
    
    //week4 added
	//netdevice 컨테이너를 만들어서 net device를 위에서 설정한 특성으로 노드컨테니어에 설치한다.
	//NetDeviceContainer devices_1;
	//devices_1 = p2p_1.Install(nc1);
    //devices_1 = p2p.Install(nc_1); //week7 added

	//NetDeviceContainer devices_2;
	//devices_2 = p2p_2.Install(nc2);
    //devices_2 = p2p.Install(nc_2); //week7 added
    
    NetDeviceContainer devices;
    devices = p2p.Install(nodes);

    //week7 added
    //p2p.EnablePcapAll("2017314585");

    //week4 added
	//노드 컨테이너에 인터넷 스택 추가
	InternetStackHelper stack;
	stack.Install(nodes);

	//IP주소도 노드컨테이너에 추가.
	Ipv4AddressHelper addr1;//, addr2;
	addr1.SetBase("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer interfaces_1 = addr1.Assign(devices);//_1);
	
	//addr2.SetBase("10.1.2.0", "255.255.255.0");
	//Ipv4InterfaceContainer interfaces_2 = addr2.Assign(devices_2);
    
    uint16_t port = 9;
	//노드 위에 올릴 apllication 구�
    UdpClientHelper client(interfaces_1.GetAddress(1), 9);
    client.SetAttribute("MaxPackets", UintegerValue(15000));
    client.SetAttribute("Interval", TimeValue(Seconds(0.001)));
	client.SetAttribute("PacketSize", UintegerValue(1024));
    
    ApplicationContainer clientApps;
    clientApps.Add(client.Install(nodes.Get(0)));
    clientApps.Start(Seconds(1.0));
    clientApps.Start(Seconds(10.0));


    //UdpEchoClientHelper echoClient_1(interfaces_1.GetAddress(1), 9); //(dest ip, dest port)
	//echoClient_1.SetAttribute("MaxPackets", UintegerValue(1500));
	//echoClient_1.SetAttribute("Interval", TimeValue(Seconds(0.001)));
	//echoClient_1.SetAttribute("PacketSize", UintegerValue(1050));

	//UdpEchoClientHelper echoClient_2(interfaces_2.GetAddress(1), 9); //(remote ip, remote port)
	//echoClient_2.SetAttribute("MaxPackets", UintegerValue(1500));
	//echoClient_2.SetAttribute("Interval", TimeValue(Seconds(0.01)));
	//echoClient_2.SetAttribute("PacketSize", UintegerValue(1050));

	//ApplicationContainer clientApps;
	//clientApps.Add(echoClient_1.Install(nc1.Get(0)));
	//clientApps.Add(echoClient_2.Install(nc2.Get(0)));
	//clientApps.Start(Seconds(2.0));
	//clientApps.Stop(Seconds(4.0));
    
    //week7 added
    //ApplicationContainer clientApps1, clientApps2;
	//clientApps1.Add(echoClient_1.Install(nc1.Get(0)));
    //clientApss2.Add(echoClient_1.Install(nc2.Get(0)));
    //clientApps1.Start(Seconds(2.0));
    //clientApps2.Start(Seconds(2.0));
    //clientApps1.Stop(Seconds(4.0));
    //clientApps2.Stop(Seconds(4.0));


    UdpServerHelper Server(port);
    ApplicationContainer ServerApps(Server.Install(nodes.Get(1)));
    ServerApps.Start(Seconds(1.0));
    ServerApps.Stop(Seconds(5.0));

    //UdpEchoServerHelper echoServer1(9);
    //UdpEchoServerHelper echoServer2(9);
	//ApplicationContainer serverApps_1(echoServer1.Install(nc1.Get(1)));
	//ApplicationContainer serverApps_2(echoServer2.Install(nc2.Get(1)));
	//serverApps_1.Start(Seconds(1.0));
	//serverApps_1.Stop(Seconds(5.0));
    
    //serverApps_2.Start(Seconds(1.0));
    //serverApps_2.Start(Seconds(5.0));

    p2p.EnablePcapAll("2017314585");
	Simulator::Run();
	Simulator::Stop(Seconds(12.0));
	Simulator::Destroy();

	return 0;
}

