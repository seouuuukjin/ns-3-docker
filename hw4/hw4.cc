#include <fstream>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("hw4");

// static void
// Counter(uint32_t prev, uint32_t now){
//     NS_LOG_INFO("Prev: " << prev << " Now: " << now << " packets at " << Simulator::Now().GetSeconds());
// }
int 
main (int argc, char *argv[])
{
    LogComponentEnable("hw4", LOG_LEVEL_ALL);
    LogComponentEnable("UdpReliableEchoClientApplication", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME));
    LogComponentEnable("UdpReliableEchoServerApplication", (LogLevel)(LOG_LEVEL_INFO|LOG_PREFIX_TIME));

    NS_LOG_INFO("hw4 Start");

    // Create nodes
    Ptr<Node> nSrcUDP = CreateObject<Node> ();
    Ptr<Node> nOnOff = CreateObject<Node> ();
    Ptr<Node> nRouter = CreateObject<Node> ();
    Ptr<Node> nDst = CreateObject<Node> (); 

    // std::cout << "1" << std::endl;

    NodeContainer nodes = NodeContainer (nSrcUDP, nOnOff, nRouter, nDst);   
    NodeContainer nSrcUDP_nRouter = NodeContainer(nSrcUDP, nRouter);
    NodeContainer nOnOff_nRouter = NodeContainer(nOnOff, nRouter);
    NodeContainer nRouter_nDst  = NodeContainer(nRouter, nDst); 

    // Create P2P channels
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
    p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1500B"));

    // std::cout << "2" << std::endl;

    NetDeviceContainer dSrcUDP_dRouter = p2p.Install (nSrcUDP, nRouter);
    NetDeviceContainer dOnOff_dRouter = p2p.Install (nOnOff, nRouter);
    NetDeviceContainer dRouter_dDst  = p2p.Install (nRouter, nDst);  

    InternetStackHelper stack;
    stack.Install (nodes); 

    // Add IP addresses
    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer iSrcUDP_iRouter = ipv4.Assign (dSrcUDP_dRouter);
    ipv4.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer iOnOff_iRouter = ipv4.Assign (dOnOff_dRouter);
    ipv4.SetBase ("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer iRouter_iDst = ipv4.Assign (dRouter_dDst);   

    // std::cout << "3" << std::endl;

    // Set up the routing tables
    Ipv4GlobalRoutingHelper::PopulateRoutingTables (); 

    // std::cout << "4" << std::endl;

    // Implement TCP & UDP sinks to the destinations
    uint16_t sinkPortOnOff = 8080;
    uint16_t sinkPortUdp = 9090;
    Address sinkAddressOnOff (InetSocketAddress (iRouter_iDst.GetAddress (1), sinkPortOnOff));

    // std::cout << "5" << std::endl;
//==========================================================================================
// Install application to the destination server
      
    //Sink for Onoff Source
    PacketSinkHelper packetSinkHelperOnOff("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sinkPortOnOff));
    ApplicationContainer sinkAppOnOff = packetSinkHelperOnOff.Install(nRouter_nDst.Get(1));
    sinkAppOnOff.Start (Seconds (0.));
    sinkAppOnOff.Stop (Seconds (31.));

    //Sink for UDP Source

    // UdpEchoServerHelper echoServer(sinkPortUdp);
    // ApplicationContainer serverApp(echoServer.Install(nRouter_nDst.Get(1)));
    // serverApp.Start(Seconds(0.0));
    // serverApp.Stop(Seconds(31.0));

    UdpReliableEchoServerHelper echoServer(sinkPortUdp);
    ApplicationContainer serverApp(echoServer.Install(nRouter_nDst.Get(1)));
    serverApp.Start(Seconds(0.0));
    serverApp.Stop(Seconds(31.0));

    // std::cout << "6" << std::endl;

//==========================================================================================
// Implement OnOff Client application

    OnOffHelper onoff("ns3::UdpSocketFactory", sinkAddressOnOff);

    // std::cout << "6.1" << std::endl;

    onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));

    // std::cout << "6.2" << std::endl;

    onoff.SetAttribute("DataRate", DataRateValue(10000000)); // 1000만
    onoff.SetAttribute("PacketSize", UintegerValue(512));
  
    // std::cout << "6.5" << std::endl;

    ApplicationContainer sourceAppOnOff = onoff.Install(nOnOff);
    sourceAppOnOff.Start (Seconds (1.));
    sourceAppOnOff.Stop (Seconds (30.));

    // std::cout << "7" << std::endl;

//==========================================================================================
// Implement UDP Client application

    // UdpEchoClientHelper echoClient(iRouter_iDst.GetAddress(1), sinkPortUdp);
	// echoClient.SetAttribute("MaxPackets", UintegerValue(1000000)); //100만
	// echoClient.SetAttribute("Interval", TimeValue(Seconds(0.01)));
	// echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    UdpReliableEchoClientHelper echoClient(iRouter_iDst.GetAddress(1), sinkPortUdp);
	echoClient.SetAttribute("MaxPackets", UintegerValue(1000000)); //100만
	echoClient.SetAttribute("Interval", TimeValue(Seconds(0.01)));
	echoClient.SetAttribute("PacketSize", UintegerValue(1024));

	ApplicationContainer sourceAppUdp = echoClient.Install(nSrcUDP);
    // sourceAppUdp.Get(0)->TraceConnectWithoutContext("SentPackets", MakeCallback(&Counter));
	sourceAppUdp.Start (Seconds (1.));
	sourceAppUdp.Stop (Seconds (30.));

    // std::cout << "8" << std::endl;
//==========================================================================================

    Simulator::Stop (Seconds (33));
    Simulator::Run ();
    Simulator::Destroy ();



}

