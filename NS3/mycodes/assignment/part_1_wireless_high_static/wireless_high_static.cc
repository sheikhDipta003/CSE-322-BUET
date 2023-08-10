/*
A single bottleneck dumbbell topology needs to be built in case of the static wireless network as shown in
the figure where the middle connection needs to be a wired one with a lower data rate. The nodes in the left
network are senders and the right ones are receivers.

            s0----        -----r0
  senders - s1---ap0 --- ap1---r1 - receivers
            s2----        -----r2

The parameters that need to be varied in the simulations are:
• Number of nodes: 20, 40, 60, 80, 100.
• Number of flows: 10, 20, 30, 40, and 50(Not considering the Ack flows)
• Number of packets per second: 100, 200, 300, 400, 500
• Coverage area : 1/2/4/5 * Tx range.

In all cases, you need to measure the average of the following metrics and plot graphs :
1. Network throughput : The amount of total bits received per second in a network.
2. Packet delivery ratio : # of packets received / # of packets delivered

Implementation guidelines:
- Consider the packet size to be 1024 bytes.
- For the coverage area, you may take the default value of Tx range to be 5.
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

// variables for output measurement
float AvgThroughput = 0;
uint32_t SentPackets = 0;
uint32_t ReceivedPackets = 0;
uint32_t ReceivedBytes = 0;

// calculate metrics
void
RxCount(Ptr< const Packet > packet, const Address &address)
{
  ReceivedPackets++;
  ReceivedBytes += packet->GetSize();
}

void
TxCount(Ptr< const Packet > packet)
{
  SentPackets++;
}

int
main(int argc, char *argv[])
{
  uint32_t payloadSize = 1024;		//bytes

  int tx_range = 5;
  std::string bottleNeckDelay = "2ms";
  std::string file = "./scratch/assignment/part_1_wireless_high_static/plots/data.txt";

  // changes for part-1
  int nNodes = 20;
  int nFlows = 10;
  int nPacketsPerSecond = 500;
  int coverageArea = 5;

  int simulationTimeInSeconds = 25;

  // input from CMD
  CommandLine cmd(__FILE__);
  cmd.AddValue("nNodes","Number of total nodes", nNodes);
  cmd.AddValue("nFlows","Number of total flows", nFlows);
  cmd.AddValue("nPacketsPerSecond","Number of packets per second", nPacketsPerSecond);
  cmd.AddValue("coverageArea","Static coverage area", coverageArea);

  cmd.AddValue("file","File to store data", file);
  cmd.Parse(argc,argv);
  
  int nLeaf = nNodes/2;
  int dataRate =(payloadSize * nPacketsPerSecond * 8) / 1000;		//kbps
  coverageArea *= tx_range;
  std::string senderDataRate = std::to_string(dataRate) + "Kbps";
  std::string bottleNeckDataRate = std::to_string(dataRate / 10) + "Kbps";

  NS_LOG_UNCOND("Using nodes : "<<nNodes<<" ; flows : "<<nFlows<<" ; packets per sec : "<<nPacketsPerSecond<<" ; coverage area : "<<coverageArea<<" ; sender data rate : "<<senderDataRate<<" ; bottleneck data rate : "<<bottleNeckDataRate);

  // config some default values
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(payloadSize));
  Config::SetDefault("ns3::RangePropagationLossModel::MaxRange", DoubleValue(coverageArea));

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
  double dx = 5.0, dy = 5.0;
  int gridCols = static_cast<int>(coverageArea / dx) + 1;
  mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                "MinX", DoubleValue(0.0),
                                "MinY", DoubleValue(0.0),
                                "DeltaX", DoubleValue(dx),
                                "DeltaY", DoubleValue(dy),
                                "GridWidth", UintegerValue(gridCols),
                                "LayoutType", StringValue("RowFirst"));
  
  // since the task is to construct wireless high-rate "static" network
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(senderWifiStaNodes);
  mobility.Install(senderWifiApNode);
  mobility.Install(receiverWifiStaNodes);
  mobility.Install(receiverWifiApNode);

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
    sinkApps.Stop(Seconds(simulationTimeInSeconds));
    Ptr<PacketSink> sink = StaticCast<PacketSink>(sinkApps.Get(0));
    sink->TraceConnectWithoutContext("Rx", MakeCallback(&RxCount));

    // setup source
    OnOffHelper sender_helper("ns3::TcpSocketFactory", (InetSocketAddress(receiverStaInterfaces.GetAddress(receiver_no), sp)));
    sender_helper.SetAttribute("PacketSize", UintegerValue(payloadSize));
    sender_helper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    sender_helper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    sender_helper.SetAttribute("DataRate", DataRateValue(DataRate(senderDataRate)));
    ApplicationContainer senderApp = sender_helper.Install(senderWifiApNode.Get(0));
    senderApp.Start(Seconds(1.0));
    senderApp.Stop(Seconds(simulationTimeInSeconds));
    Ptr<OnOffApplication> src = StaticCast<OnOffApplication>(senderApp.Get(0));
    src->TraceConnectWithoutContext("Tx", MakeCallback(&TxCount));

    NS_LOG_UNCOND(sender_no<<" <------> "<<receiver_no << " : " <<senderStaInterfaces.GetAddress(sender_no)<<" : "<<receiverStaInterfaces.GetAddress(receiver_no));

    // boundary cases
    sender_no += 1;
    if(sender_no == nLeaf) {
      sender_no = 0;
      receiever_inc += 1;
    }
  }

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Simulator::Stop(Seconds(simulationTimeInSeconds));
  Simulator::Run();

  // /////////////////////// COLLECT AND STORE STATS ///////////////////////
  
  AvgThroughput = ReceivedBytes*8.0 /((Simulator::Now().GetSeconds())*1000);
  NS_LOG_UNCOND("\n--------Total Results of the simulation----------"<<std::endl);
  NS_LOG_UNCOND("Total sent packets  = " << SentPackets);
  NS_LOG_UNCOND("Total Received Packets = " << ReceivedPackets);
  NS_LOG_UNCOND("Average Throughput = " << AvgThroughput<< "Kbps");
  NS_LOG_UNCOND("Packet Delivery Ratio = " <<((ReceivedPackets*100.00)/SentPackets)<< "%");
  
  std::ofstream MyFile(file, std::ios_base::app);

  // first x values
  MyFile << nNodes << " " << nFlows << " " << nPacketsPerSecond << " " << coverageArea  << " ";
  // then y values
  MyFile << AvgThroughput << " " <<((ReceivedPackets*100.00)/SentPackets) <<std::endl;

  MyFile.close();

  Simulator::Destroy();

  return 0;
}