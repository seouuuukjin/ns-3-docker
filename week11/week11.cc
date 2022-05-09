#include <fstream>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/bridge-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

// You can waf this with command <./waf --run scratch/week11/week11 > week11-log.dat>
NS_LOG_COMPONENT_DEFINE ("week11");

static void
Rxtime (std::string context, Ptr<const Packet> p, const Address &a){

    static double bytes1, bytes2, bytes_background= 0;

    if (context == "Flow1"){
    //   printf("flow 11111 event\n");
        bytes1 += p->GetSize();
        // // NS_LOG_UNCOND : logging module that logs unconditionally
        // NS_LOG_UNCOND("1\t" << Simulator::Now().GetSeconds()
        //      << "\t" << bytes1 * 8 / 1000000 / (Simulator::Now().GetSeconds()-1));
        std::cout << "1\t" << Simulator::Now().GetSeconds()
             << "\t" << bytes1 * 8 / 1000000 / (Simulator::Now().GetSeconds()-1) << std::endl;
    }
    else if(context == "Flow2"){
      // printf("flow 22222 event\n");
        bytes2 += p->GetSize();
        // // NS_LOG_UNCOND("2\t" << Simulator::Now().GetSeconds()
        //      << "\t" << bytes2 * 8 / 1000000 / (Simulator::Now().GetSeconds()-3));
        std::cout << "2\t" << Simulator::Now().GetSeconds()
             << "\t" << bytes2 * 8 / 1000000 / (Simulator::Now().GetSeconds()-1) << std::endl;
    }
    else if(context == "Background Flow"){
      // printf("flow background event\n");
        bytes_background += p->GetSize();
        // // NS_LOG_UNCOND("2\t" << Simulator::Now().GetSeconds()
        //      << "\t" << bytes2 * 8 / 1000000 / (Simulator::Now().GetSeconds()-3));
        std::cout << "0\t" << Simulator::Now().GetSeconds()
             << "\t" << bytes_background * 8 / 1000000 / (Simulator::Now().GetSeconds()-1) << std::endl;
    }
    // printf("RXtime called @@@@@\n");
}

int 
main (int argc, char *argv[])
{
  // LogComponentEnable("OnOffApplication", LOG_);
  // LogComponentEnable("PacketSink", LOG_LEVEL_INFO);

  CommandLine cmd;
  cmd.Parse(argc,argv);

  // Create nodes
  NodeContainer nodes;
  nodes.Create(4);

  // Node container for Switch
  NodeContainer csmaSwitch;
  csmaSwitch.Create(1);

  // Create P2P channels (link)
  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
  csma.SetChannelAttribute("Delay", TimeValue(MicroSeconds(10)));

  // Create net divice
  NetDeviceContainer csmaHostDevices;
  NetDeviceContainer switchDevices;
  
  // Connect the switch node to all host node
  for(int i=0; i<4; i++){
      NetDeviceContainer link = csma.Install(NodeContainer(nodes.Get(i), csmaSwitch));
      csmaHostDevices.Add(link.Get(0));
      switchDevices.Add(link.Get(1));
  }
  
  // Install the Bridge Functionality to Switch
  Ptr<Node> switchNode = csmaSwitch.Get(0);
  BridgeHelper bridge;
  bridge.Install(switchNode, switchDevices);

  // Install the internet stack. The stack can be installed at node ONLY ONCE.
  InternetStackHelper stack;
  stack.Install (nodes);

  // Add IP addresses
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  ipv4.Assign (csmaHostDevices);

  ////////////////////////////////////////////////
  // Background flow : n2->n1
  // Set the mission source node (n2) will do
  uint16_t port = 9;
  OnOffHelper onoff("ns3::UdpSocketFactory", 
    Address(InetSocketAddress(Ipv4Address("10.1.1.2"), port)));
  onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]") );
  onoff.SetAttribute("OffTime",	StringValue("ns3::ConstantRandomVariable[Constant=0.0]") );
  onoff.SetAttribute("DataRate", DataRateValue(5000000)); // = 5Mb/s

  ApplicationContainer app1 = onoff.Install(nodes.Get(2));
  app1.Start(Seconds(1.0));
  app1.Stop(Seconds(16.0));
  
  PacketSinkHelper sink_background_flow("ns3::UdpSocketFactory", 
    Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
  ApplicationContainer sinkApp_background = sink_background_flow.Install(nodes.Get(1));
  sinkApp_background.Start(Seconds(0.5));
  sinkApp_background.Get(0)->TraceConnect("Rx", "Background Flow", MakeCallback (&Rxtime));
  
  ////////////////////////////////////////////////
  // Flow1 : n0 -> n1
  uint16_t port_flow1 = 111;
  OnOffHelper onoff_flow1("ns3::UdpSocketFactory", 
    Address(InetSocketAddress(Ipv4Address("10.1.1.2"), port_flow1)));
//   onoff.SetAttribute("Remote", AddressValue(InetSocketAddress(Ipv4Address("10.1.1.2"), port)));
  onoff_flow1.SetAttribute("DataRate", DataRateValue(2500000));
  onoff_flow1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]") );
  onoff_flow1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]") );

  ApplicationContainer app2 = onoff_flow1.Install(nodes.Get(0));
  app2.Start(Seconds(1.0));
  app2.Stop(Seconds(16.0));

  PacketSinkHelper sink_flow1("ns3::UdpSocketFactory", 
    Address(InetSocketAddress(Ipv4Address::GetAny(), port_flow1)));
  ApplicationContainer sinkApp_flow1 = sink_flow1.Install(nodes.Get(1));
  sinkApp_flow1.Start(Seconds(0.5));
  sinkApp_flow1.Get(0)->TraceConnect("Rx", "Flow1", MakeCallback (&Rxtime));

  ////////////////////////////////////////////////
  // Flow2 : n3 -> n0
  OnOffHelper onoff_flow2("ns3::UdpSocketFactory", 
    Address(InetSocketAddress(Ipv4Address("10.1.1.1"), port)));
//   onoff.SetAttribute("Remote", AddressValue(InetSocketAddress(Ipv4Address("10.1.1.1"), port)));
  onoff_flow2.SetAttribute("DataRate", DataRateValue(5000000));
  onoff_flow2.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]") );
  onoff_flow2.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]") );

  ApplicationContainer app3 = onoff_flow2.Install(nodes.Get(3));
  app3.Start(Seconds(6.0));
  app3.Stop(Seconds(20.0));

  PacketSinkHelper sink_flow2("ns3::UdpSocketFactory", 
    Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
  ApplicationContainer sinkApp_flow2 = sink_flow2.Install(nodes.Get(0));
  sinkApp_flow2.Start(Seconds(5.0));
  sinkApp_flow2.Get(0)->TraceConnect("Rx", "Flow2", MakeCallback (&Rxtime));


  // All pair setting end
  ////////////////////////////////////////////////
  
  // option : Enable Pcap tracing
  csma.EnablePcapAll("week11-hw", false);

  // Set up the routing tables (Only need this when using Router, not Switch)
  // Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (20));
  Simulator::Run ();
  Simulator::Destroy ();



}

