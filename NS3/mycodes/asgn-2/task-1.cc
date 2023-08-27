#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/applications-module.h"
#include "ns3/stats-module.h"
#include "ns3/callback.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/csma-module.h"

using namespace ns3;

// ===========================================================================
//            s0----           -----r0
//  senders - s1---- R0 --- R1 ----r1 - receivers
// ===========================================================================
//

class PcktSender : public Application
{
public:
  PcktSender();
  ~PcktSender() override;

  /**
   * Register this type.
   * \return The TypeId.
   */
  static TypeId GetTypeId(void);
  void Setup(Ptr<Socket> socket, Address address, uint32_t packetSize, DataRate dataRate, uint32_t simultime);

private:
  void StartApplication(void) override;
  void StopApplication(void) override;

  void ScheduleTx(void);
  void SendPacket(void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
  uint32_t        m_simultime;
};

PcktSender::PcktSender()
  : m_socket(0),
    m_peer(),
    m_packetSize(0),
    m_dataRate(0),
    m_sendEvent(),
    m_running(false),
    m_packetsSent(0),
    m_simultime(0)
{
}

PcktSender::~PcktSender()
{
  m_socket = 0;
}

/* static */
TypeId PcktSender::GetTypeId(void)
{
  static TypeId tid = TypeId("PcktSender")
    .SetParent<Application>()
    .SetGroupName("Tutorial")
    .AddConstructor<PcktSender>()
    ;
  return tid;
}

void
PcktSender::Setup(Ptr<Socket> socket, Address address, uint32_t packetSize, DataRate dataRate, uint32_t simultime)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_dataRate = dataRate;
  m_simultime = simultime;
}

void
PcktSender::StartApplication(void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind();	
  m_socket->Connect(m_peer);	
  SendPacket();
}

void
PcktSender::StopApplication(void)
{
  m_running = false;

  if(m_sendEvent.IsRunning())
    {
      Simulator::Cancel(m_sendEvent);
    }

  if(m_socket)
    {
      m_socket->Close();
    }
}

void
PcktSender::SendPacket(void)
{
  Ptr<Packet> packet = Create<Packet>(m_packetSize);
  m_socket->Send(packet);

  if(Simulator::Now().GetSeconds() < m_simultime) ScheduleTx();
}

void
PcktSender::ScheduleTx(void)
{
  if(m_running)
    {
      Time tNext(Seconds(m_packetSize * 8 / static_cast<double>(m_dataRate.GetBitRate())));
      m_sendEvent = Simulator::Schedule(tNext, &PcktSender::SendPacket, this);
    }
}

static void
CwndChange(Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream() << Simulator::Now().GetSeconds() << " " << newCwnd << std::endl;
}

int main(int argc, char *argv[]){
    uint32_t payloadSize = 1472;
    std::string tcp1 = "ns3::TcpNewReno"; // TcpNewReno
    std::string tcp2 = "ns3::TcpHighSpeed"; //TcpAdaptiveReno, TcpWestwoodPlus, TcpHighSpeed
    std::string senderDataRate = "1Gbps";
    std::string senderDelay = "1ms";
    int bttlnkRate = 50;
    int bttlnkDelay = 100;
    std::string results = "scratch/asgn-2/results/tcphs";
    int simulTime = 60;
    int pcktLossExp = 6;
    int flag = 1;

    // input from CMD
    CommandLine cmd(__FILE__);
    cmd.AddValue("tcp2","Name of TCP variant 2", tcp2);
    cmd.AddValue("bttlnkRate","Datarate of the bottlelink", bttlnkRate);
    cmd.AddValue("pcktLossExp", "Packet loss rate of the error model", pcktLossExp);
    cmd.AddValue("results","Folder to store data andd plots", results);
    cmd.AddValue("experimentNo","What to vary: 1 for bttlnkRate, 2 for pcktLossExp", flag);
    cmd.Parse(argc,argv);

    std::string file = results + "/data_" + std::to_string(flag) + ".txt";
    double packet_loss_rate =(1.0 / std::pow(10, pcktLossExp));
    std::string bottleNeckDataRate = std::to_string(bttlnkRate) + "Mbps";
    std::string bottleNeckDelay = std::to_string(bttlnkDelay) + "ms";

    NS_LOG_UNCOND("USING TCP 1 = "<< tcp1 <<" ; TCP 2 = "<< tcp2<< " ; bottleneck rate = "<< bottleNeckDataRate <<" ; packet loss rate = "<< packet_loss_rate);

    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(payloadSize));

    // setup nodes, devices and channels
    PointToPointHelper p2pBottleneck, p2pLeaf;

    p2pBottleneck.SetDeviceAttribute ("DataRate", StringValue(bottleNeckDataRate));
    p2pBottleneck.SetChannelAttribute("Delay", StringValue(bottleNeckDelay));

    p2pLeaf.SetDeviceAttribute ("DataRate", StringValue(senderDataRate));
    p2pLeaf.SetChannelAttribute("Delay", StringValue(senderDelay));

    // add 'router buffer capacity' equal to 'bandwidth-delay product'
    p2pLeaf.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue(std::to_string(bttlnkDelay * bttlnkRate) + "p"));

    PointToPointDumbbellHelper p2pDumbbell(2, p2pLeaf, 2, p2pLeaf, p2pBottleneck);

    // add error rate
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    em->SetAttribute("ErrorRate", DoubleValue(packet_loss_rate));
    p2pDumbbell.m_routerDevices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em)); 

    // install stack
    // tcp variant 1
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue(tcp1));
    InternetStackHelper stack1;
    stack1.Install(p2pDumbbell.GetLeft(0));
    stack1.Install(p2pDumbbell.GetRight(0));
    stack1.Install(p2pDumbbell.GetLeft());
    stack1.Install(p2pDumbbell.GetRight());

    // tcp variant 2
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue(tcp2));
    InternetStackHelper stack2;
    stack2.Install(p2pDumbbell.GetLeft(1));
    stack2.Install(p2pDumbbell.GetRight(1));

    // assign IP addr
    p2pDumbbell.AssignIpv4Addresses(Ipv4AddressHelper("10.1.1.0", "255.255.255.0"), // left nodes
                          Ipv4AddressHelper("10.2.1.0", "255.255.255.0"),  // right nodes
                          Ipv4AddressHelper("10.3.1.0", "255.255.255.0")); // routers 
    
    Ipv4GlobalRoutingHelper::PopulateRoutingTables(); // populate routing table

    // install flow monitor
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    uint16_t sp = 8000;
    for(int i=0;i<2; i++){
      Address sinkAddr(InetSocketAddress(p2pDumbbell.GetRightIpv4Address(i), sp));
      PacketSinkHelper sinkHelp("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sp));
      ApplicationContainer sinkApp = sinkHelp.Install(p2pDumbbell.GetRight(i));
      sinkApp.Start(Seconds(0));
      sinkApp.Stop(Seconds(simulTime));

      Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(p2pDumbbell.GetLeft(i), TcpSocketFactory::GetTypeId());
      Ptr<PcktSender> srcApp = CreateObject<PcktSender>();
      srcApp->Setup(ns3TcpSocket, sinkAddr, payloadSize, DataRate(senderDataRate), simulTime);
      p2pDumbbell.GetLeft(i)->AddApplication(srcApp);
      srcApp->SetStartTime(Seconds(1));
      srcApp->SetStopTime(Seconds(simulTime));

      std::ostringstream oss;
      oss << results << "/flow" << i+1 <<  ".dat";
      AsciiTraceHelper asciiTraceHelper;
      Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream(oss.str());
      ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow", MakeBoundCallback(&CwndChange, stream));
    }
  
    Simulator::Stop(Seconds(simulTime));
    Simulator::Run();

    // flow monitor statistics
    int j = 0;
    double thrput_curr = 0;
    double thrputAll[2] = {0};
    double jainIndexCalc[2] = {0.0};

    std::ofstream statsFile(file, std::ios_base::app);

    // variables for output measurement
    uint32_t SentPackets = 0;
    uint32_t ReceivedPackets = 0;
    uint32_t LostPackets = 0;

    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

    for(auto iter = stats.begin(); iter != stats.end(); ++iter) {
      thrput_curr = iter->second.rxBytes * 8.0/((simulTime)*1000);
      if(j%2 == 0) { thrputAll[0] += iter->second.rxBytes; }
      if(j%2 == 1) { thrputAll[1] += iter->second.rxBytes; }

      SentPackets = SentPackets +(iter->second.txPackets);
      ReceivedPackets = ReceivedPackets +(iter->second.rxPackets);
      LostPackets = LostPackets +(iter->second.lostPackets);
      j = j + 1;

      jainIndexCalc[0] += thrput_curr;
      jainIndexCalc[1] +=(thrput_curr * thrput_curr);
    }

    double jain_index =(jainIndexCalc[0] * jainIndexCalc[0]) /( j * jainIndexCalc[1]);
    thrputAll[0] /=(simulTime * 1000);
    thrputAll[1] /=(simulTime * 1000);

    statsFile << bottleNeckDataRate.substr(0, bottleNeckDataRate.length()-4) << " " << -1 * pcktLossExp << " " << thrputAll[0] << " " << thrputAll[1] << " " << jain_index <<std::endl;
    
    Simulator::Destroy();

    return 0;
}