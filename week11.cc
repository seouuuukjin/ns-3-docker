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

    static double bytes1, bytes2 = 0;

    if (context == "Flow1"){
      // printf("flow 11111 event\n");
        bytes1 += p->GetSize();
        // NS_LOG_UNCOND : logging module that logs unconditionally
        NS_LOG_UNCOND("1\t" << Simulator::Now().GetSeconds()
             << "\t" << bytes1 * 8 / 1000000 / (Simulator::Now().GetSeconds()-1));
        std::cout << "1\t" << Simulator::Now().GetSeconds()
             << "\t" << bytes1 * 8 / 1000000 / (Simulator::Now().GetSeconds()-1) << std::endl;
    }
    else if(context == "Flow2"){
      // printf("flow 22222 event\n");
        bytes2 += p->GetSize();
        NS_LOG_UNCOND("2\t" << Simulator::Now().GetSeconds()
             << "\t" << bytes1 * 8 / 1000000 / (Simulator::Now().GetSeconds()-3));
        std::cout << "1\t" << Simulator::Now().GetSeconds()
             << "\t" << bytes1 * 8 / 1000000 / (Simulator::Now().GetSeconds()-1) << std::endl;
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
  // Ptr<Node> node0 = CreateObject<Node> ();
  // Ptr<Node> node1 = CreateObject<Node> ();
  // Ptr<Node> node2 = CreateObject<Node> ();
  // Ptr<Node> node3 = CreateObject<Node> ();
  // NodeContainer nodes = NodeContainer (node0, node1, node2, node3);
  NodeContainer nodes;
  nodes.Create(4);

//   NodeContainer node0to1 = NodeContainer(node0, node1);
//   NodeContainer node2to3 = NodeContainer(node2, node3);

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

  uint16_t port = 9;
  OnOffHelper onoff("ns3::UdpSocketFactory", 
    Address(InetSocketAddress(Ipv4Address("10.1.1.2"), port)));
  onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]") );
  onoff.SetAttribute("OffTime",	StringValue("ns3::ConstantRandomVariable[Constant=1.0]") );
  onoff.SetAttribute("DataRate", DataRateValue(5000000)); // = 5Mb/s

  ApplicationContainer app1 = onoff.Install(nodes.Get(0));
  app1.Start(Seconds(1.0));
  app1.Stop(Seconds(10.0));
  
  PacketSinkHelper sink("ns3::UdpSocketFactory", 
    Address(InetSocketAddress(Ipv4Address::GetAny(), port)));

  ApplicationContainer sinkApp1 = sink.Install(nodes.Get(1));
  sinkApp1.Start(Seconds(1.0));
  sinkApp1.Get(0)->TraceConnect("Rx", "Flow1", MakeCallback (&Rxtime));

  onoff.SetAttribute("Remote", AddressValue(InetSocketAddress(Ipv4Address("10.1.1.1"), port)));
  onoff.SetAttribute("DataRate", DataRateValue(10000000));
  onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=0.3]") );
  onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.7]") );

  ApplicationContainer app2 = onoff.Install(nodes.Get(3));
  app2.Start(Seconds(3.0));
  app2.Stop(Seconds(13.0));

  ApplicationContainer sinkApp2 = sink.Install(nodes.Get(0));
  sinkApp2.Start(Seconds(3.0));
  sinkApp2.Get(0)->TraceConnect("Rx", "Flow2", MakeCallback(&Rxtime));


  // option : Enable Pcap tracing
  csma.EnablePcapAll("week11", false);

  // Set up the routing tables
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (15));
  Simulator::Run ();
  Simulator::Destroy ();



}

