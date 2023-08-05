/*
A single bottleneck dumbbell topology needs to be built in case of the mobile wireless network as shown in
the figure. The nodes in the left network are senders and the right ones are receivers.

            s0----        -----r0
  senders - s1---ap0 --- ap1---r1 - receivers
            s2----        -----r2

The parameters that need to be varied in the simulations are:
• Number of nodes: 20, 40, 60, 80, 100.
• Number of flows: 10, 20, 30, 40, and 50(Not considering the Ack flows)
• Number of packets per second: 100, 200, 300, 400, 500
• Speed of nodes: 5 m/s, 10 m/s, 15 m/s, 20 m/s, 25 m/s

In all cases, you need to measure the average of the following metrics and plot graphs :
1. Network throughput : The amount of total bits received per second in a network.
2. Packet delivery ratio : # of packets received / # of packets delivered

Implementation guidelines:
- Consider the packet size to be 1024 bytes.
- The graphs need to be plotted for the average performance metrics of the varying parameters(such as
flow VS throughput, speed VS Packet Delivery Ratio etc). 
*/

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/yans-error-rate-model.h"

#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/dsr-module.h"

using namespace ns3;

// print positions
void getNodePos(NodeContainer container){
  for(NodeContainer::Iterator j = container.Begin(); j != container.End(); j++)
    {
      Ptr<Node> object = *j;
      Ptr<MobilityModel> pos = object->GetObject<MobilityModel>();
      NS_ASSERT(pos != 0);
      Vector vect = pos->GetPosition();
      std::cout << "x=" << vect.x << ", y=" << vect.y << ", z=" << vect.z << std::endl;
    }
}

int
main(int argc, char *argv[])
{
  uint32_t payloadSize = 1024;		//bytes

  std::string bottleNeckDelay = "2ms";
  std::string tcpVariant = "TcpNewReno"; /* TCP variant type. */
  std::string file = "./scratch/assignment/part_2_wireless_high_mobile/plots/data.txt";

  // changes for part-1
  int nNodes = 20;
  int nFlows = 20;
  int nPacketsPerSecond = 500;
  int speed = 5;

  int simulationTimeInSeconds = 25;
  int cleanupTime = 2;

  // input from CMD
  CommandLine cmd(__FILE__);
  cmd.AddValue("nNodes","Number of total nodes", nNodes);
  cmd.AddValue("nFlows","Number of total flows", nFlows);
  cmd.AddValue("nPacketsPerSecond","Number of packets per second", nPacketsPerSecond);
  cmd.AddValue("speed","Speed of the mobile Station Nodes", speed);

  cmd.AddValue("file","File to store data", file);
  cmd.Parse(argc,argv);
  
  nFlows = nFlows/2;
  int nLeaf = nNodes/2;
  int dataRate =(payloadSize * nPacketsPerSecond * 8) / 1000;		//kbps
  std::string senderDataRate = std::to_string(dataRate) + "Kbps";
  std::string bottleNeckDataRate = std::to_string(dataRate / 10) + "Kbps";

  NS_LOG_UNCOND("Using nodes : "<<nNodes<<" ; flows : "<<2*nFlows<<" ; packets per sec : "<<nPacketsPerSecond<<" ; "\
                "speed : "<<speed<<" ; sender data rate : "<<senderDataRate<<" ; bottleneck data rate : "<<bottleNeckDataRate);

  // config some default values
  tcpVariant = std::string("ns3::") + tcpVariant;
  // Select TCP variant
  TypeId tcpTid;
  NS_ABORT_MSG_UNLESS(TypeId::LookupByNameFailSafe(tcpVariant, &tcpTid), "TypeId " << tcpVariant << " not found");
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TypeId::LookupByName(tcpVariant)));
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(payloadSize));

  /////////////////////// SETUP NODES ///////////////////////
  // setup ap nodes 
  NodeContainer p2pNodes;
  p2pNodes.Create(2);

  // setup sender devices
  NodeContainer senderWifiStaNodes, receiverWifiStaNodes;
  NodeContainer senderWifiApNode = p2pNodes.Get(0);
  senderWifiStaNodes.Create(nLeaf);

  //setup receivers devices
  NodeContainer receiverWifiApNode = p2pNodes.Get(1);
  receiverWifiStaNodes.Create(nLeaf);

  /////////////////////// SETUP TOPOLOGY HELPERS ///////////////////////
  
  // routers in p2p
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue(bottleNeckDataRate));
  pointToPoint.SetChannelAttribute("Delay", StringValue(bottleNeckDelay));

  NetDeviceContainer pointToPointDevies;
  pointToPointDevies = pointToPoint.Install(p2pNodes);

  // senders in wifi
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  YansWifiChannelHelper channel2 = YansWifiChannelHelper::Default();
  YansWifiPhyHelper phy;
  YansWifiPhyHelper phy2;

  channel.AddPropagationLoss("ns3::RangePropagationLossModel");
  channel2.AddPropagationLoss("ns3::RangePropagationLossModel");

  phy.SetChannel(channel.Create());
  phy2.SetChannel(channel2.Create());

  WifiHelper senderWifi;
  WifiHelper receiverWifi;

  Ssid ssid = Ssid("ns-3-ssid"); // this is 802.11

  WifiMacHelper mac;
  WifiMacHelper mac2;

  mac.SetType("ns3::StaWifiMac",
              "Ssid", SsidValue(ssid),
              "ActiveProbing", BooleanValue(false));
  mac2.SetType("ns3::StaWifiMac",
              "Ssid", SsidValue(ssid),
              "ActiveProbing", BooleanValue(false));

  // sender nodes
  NetDeviceContainer senderStaDevices;
  NetDeviceContainer receiverStaDevices;
  senderStaDevices = senderWifi.Install(phy, mac, senderWifiStaNodes);
  receiverStaDevices = receiverWifi.Install(phy2, mac2, receiverWifiStaNodes);

  mac.SetType("ns3::ApWifiMac",
              "Ssid", SsidValue(ssid));
  mac2.SetType("ns3::ApWifiMac",
              "Ssid", SsidValue(ssid));
  
  // sender ap
  NetDeviceContainer senderApDevices;
  NetDeviceContainer receiverApDevices;
  senderApDevices = senderWifi.Install(phy, mac, senderWifiApNode);
  receiverApDevices = receiverWifi.Install(phy2, mac2, receiverWifiApNode);

  //setup mobility model
  MobilityHelper mobility;

  mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                "MinX", DoubleValue(0.0),
                                "MinY", DoubleValue(0.0),
                                "DeltaX", DoubleValue(1.0),
                                "DeltaY", DoubleValue(1.0),
                                "GridWidth", UintegerValue(3),
                                "LayoutType", StringValue("RowFirst"));
  
  // tell STA nodes how to move
  mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue(Rectangle(-50, 50, -50, 50)),
                             "Speed", StringValue("ns3::ConstantRandomVariable[Constant="+std::to_string(speed)+"]"));
  
  // install on STA nodes
  mobility.Install(senderWifiStaNodes);
  mobility.Install(receiverWifiStaNodes);

  // tell AP node to stay still
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  // install on AP node
  mobility.Install(senderWifiApNode);
  mobility.Install(receiverWifiApNode);

  // iterate our nodes and print their position.
  getNodePos(senderWifiStaNodes);
  NS_LOG_UNCOND("AP -> ");
  getNodePos(senderWifiApNode);
  getNodePos(receiverWifiStaNodes);
  NS_LOG_UNCOND("AP -> ");
  getNodePos(receiverWifiApNode);

  /////////////////////// INSTALL STACK ///////////////////////
  InternetStackHelper stack1;
  stack1.Install(receiverWifiStaNodes);
  stack1.Install(receiverWifiApNode);
  stack1.Install(senderWifiStaNodes);
  stack1.Install(senderWifiApNode);
  
  /////////////////////// ASSIGN IP Addresses ///////////////////////
  Ipv4AddressHelper address;

  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign(pointToPointDevies);

  address.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer senderApInterface = address.Assign(senderApDevices);
  Ipv4InterfaceContainer senderStaInterfaces = address.Assign(senderStaDevices);

  address.SetBase("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer receiverApInterface = address.Assign(receiverApDevices);
  Ipv4InterfaceContainer receiverStaInterfaces = address.Assign(receiverStaDevices);

  /////////////////////// SETUP SOURCE AND SINK ///////////////////////
  uint16_t sp;
  NS_LOG_UNCOND("Connections : ");
  
  int sender_no=0;
  int receiver_no, receiever_inc=0;
  for(int flow=0; flow<nFlows; flow++)
  {
    sp = 8000 + flow;
    receiver_no =(sender_no + receiever_inc) % nLeaf; 

    // setup sink
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sp)); // 0.0.0.0 address 
    ApplicationContainer sinkApps = packetSinkHelper.Install(receiverWifiStaNodes.Get(receiver_no));
    sinkApps.Start(Seconds(0.));
    sinkApps.Stop(Seconds(simulationTimeInSeconds + cleanupTime));

    // setup source
    OnOffHelper sender_helper("ns3::TcpSocketFactory", (InetSocketAddress(receiverStaInterfaces.GetAddress(receiver_no), sp)));
    sender_helper.SetAttribute("PacketSize", UintegerValue(payloadSize));
    sender_helper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    sender_helper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    sender_helper.SetAttribute("DataRate", DataRateValue(DataRate(senderDataRate)));
    ApplicationContainer senderApp = sender_helper.Install(senderWifiApNode.Get(0));
    senderApp.Start(Seconds(1.0));
    senderApp.Stop(Seconds(simulationTimeInSeconds));

    NS_LOG_UNCOND(sender_no<<" <------> "<<receiver_no << " : " <<senderStaInterfaces.GetAddress(sender_no)<<" : "<<receiverStaInterfaces.GetAddress(receiver_no));

    // boundary cases
    sender_no += 1;
    if(sender_no == nLeaf) {
      sender_no = 0;
      receiever_inc += 1;
    }
  }

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  // install flow monitor
  FlowMonitorHelper flowmon;
  flowmon.SetMonitorAttribute("MaxPerHopDelay", TimeValue(Seconds(cleanupTime)));
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  Simulator::Stop(Seconds(simulationTimeInSeconds + cleanupTime));
  Simulator::Run();

  // /////////////////////// FLOW MONITOR STATUS ///////////////////////

  // variables for output measurement
  float AvgThroughput = 0;
  uint32_t SentPackets = 0;
  uint32_t ReceivedPackets = 0;
  uint32_t ReceivedBytes = 0;

  std::ofstream MyFile(file, std::ios_base::app);

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

  int j=0;
  for(auto iter = stats.begin(); iter != stats.end(); ++iter) {
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(iter->first); 
    // classifier returns FiveTuple in correspondance to a flowID

    NS_LOG_UNCOND("----Flow ID:" <<iter->first);
    NS_LOG_UNCOND("Src Addr" <<t.sourceAddress << " -- Dst Addr "<< t.destinationAddress);
    NS_LOG_UNCOND("Sent Packets = " <<iter->second.txPackets);
    NS_LOG_UNCOND("Received Packets = " <<iter->second.rxPackets);
    NS_LOG_UNCOND("Packet delivery ratio = " <<iter->second.rxPackets*100.0/iter->second.txPackets << "%");
    NS_LOG_UNCOND("Throughput = " <<iter->second.rxBytes * 8.0/((simulationTimeInSeconds+cleanupTime)*1000)<<"Kbps");
    NS_LOG_UNCOND(" ");
    SentPackets = SentPackets +(iter->second.txPackets);
    ReceivedPackets = ReceivedPackets +(iter->second.rxPackets);
    ReceivedBytes = ReceivedBytes +(iter->second.rxBytes);

    j += 1;
  }
  
  AvgThroughput = ReceivedBytes*8.0 /((simulationTimeInSeconds + cleanupTime)*1000);
  NS_LOG_UNCOND("\n--------Total Results of the simulation----------"<<std::endl);
  NS_LOG_UNCOND("Total sent packets  = " << SentPackets);
  NS_LOG_UNCOND("Total Received Packets = " << ReceivedPackets);
  NS_LOG_UNCOND("Average Throughput = " << AvgThroughput<< "Kbps");
  NS_LOG_UNCOND("Packet Delivery Ratio = " <<((ReceivedPackets*100.00)/SentPackets)<< "%");
  NS_LOG_UNCOND("Total Flows " << j);

  // first x values
  MyFile << nNodes << " " << 2*nFlows << " " << nPacketsPerSecond << " " << speed  << " ";
  // then y values
  MyFile << AvgThroughput << " " <<((ReceivedPackets*100.00)/SentPackets) <<std::endl;

  MyFile.close();

  Simulator::Destroy();

  return 0;
}
