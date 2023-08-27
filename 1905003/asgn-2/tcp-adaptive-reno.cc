#include "tcp-adaptive-reno.h"

NS_LOG_COMPONENT_DEFINE("TcpAdaptiveReno");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(TcpAdaptiveReno);

TypeId
TcpAdaptiveReno::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::TcpAdaptiveReno")
      .SetParent<TcpNewReno>()
      .SetGroupName("Internet")
      .AddConstructor<TcpAdaptiveReno>()
      .AddAttribute(
          "FilterType",
          "Use this to choose no filter or Tustin's approximation filter",
          EnumValue(TcpAdaptiveReno::TUSTIN),
          MakeEnumAccessor(&TcpAdaptiveReno::m_fType),
          MakeEnumChecker(TcpAdaptiveReno::NONE, "None", TcpAdaptiveReno::TUSTIN, "Tustin"))
      .AddTraceSource("EstimatedBW",
                      "The estimated bandwidth",
                      MakeTraceSourceAccessor(&TcpAdaptiveReno::m_currentBW),
                      "ns3::TracedValueCallback::Double");
  return tid;
}

TcpAdaptiveReno::TcpAdaptiveReno(void)
  : TcpWestwoodPlus(),
    m_rtt_min(Time(0)),
    m_rtt_curr(Time(0)),
    m_rtt_j_loss(Time(0)),
    m_rtt_j_conj(Time(0)),
    m_rtt_j1_conj(Time(0)),
    m_wnd_inc(0),
    m_wnd_base(0),
    m_wnd_probe(0)
{
  NS_LOG_FUNCTION(this);
}

TcpAdaptiveReno::TcpAdaptiveReno(const TcpAdaptiveReno& sock)
  : TcpWestwoodPlus(sock),
    m_rtt_min(Time(0)),
    m_rtt_curr(Time(0)),
    m_rtt_j_loss(Time(0)),
    m_rtt_j_conj(Time(0)),
    m_rtt_j1_conj(Time(0)),
    m_wnd_inc(0),
    m_wnd_base(0),
    m_wnd_probe(0)
{
  NS_LOG_FUNCTION(this);
  NS_LOG_LOGIC("Invoked the copy constructor");
}

TcpAdaptiveReno::~TcpAdaptiveReno(void)
{
}

uint32_t
TcpAdaptiveReno::GetSsThresh(Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight)
{
  /*
    W_base = W ∗ W_dec = W / (1 + c)
    W_probe = 0
  */

  m_rtt_j1_conj = m_rtt_j_conj;   // save the previous conjestion RTT
  m_rtt_j_loss = m_rtt_curr;      // since loss has been detected after current packet loss, save current RTT into 'm_rtt_j_loss'

  double c = EstimateCongestionLevel();

  uint32_t ssthresh = std::max( 2*tcb->m_segmentSize, (uint32_t)(tcb->m_cWnd /(1.0 + c)) );

  m_wnd_base = ssthresh;
  m_wnd_probe = 0;

  NS_LOG_LOGIC("new ssthresh : "<< ssthresh <<" ; old conj Rtt : "<< m_rtt_j1_conj <<" ; new conj Rtt : "<< m_rtt_j_conj <<" ; congestion : "<< c);

  return ssthresh;
}

void
TcpAdaptiveReno::PktsAcked(Ptr<TcpSocketState> tcb, uint32_t packetsAcked, const Time& rtt)
{
  NS_LOG_FUNCTION(this << tcb << packetsAcked << rtt);

  if(rtt.IsZero())
    {
      NS_LOG_WARN("RTT measured is zero!");
      return;
    }

  m_ackedSegments += packetsAcked;

  if(m_rtt_min.IsZero()) { m_rtt_min = rtt; }
  else if(rtt <= m_rtt_min) { m_rtt_min = rtt; }

  m_rtt_curr = rtt;
  NS_LOG_LOGIC("Minimum RTT : "<<m_rtt_min.GetMilliSeconds() << "ms");
  NS_LOG_LOGIC("Current RTT: " << m_rtt_curr.GetMilliSeconds() << "ms");

  TcpWestwoodPlus::EstimateBW(rtt, tcb);
}

Ptr<TcpCongestionOps>
TcpAdaptiveReno::Fork()
{
  return CreateObject<TcpAdaptiveReno>(*this);
}

double
TcpAdaptiveReno::EstimateCongestionLevel()
{
  /*
    RTT_cong_j = a RTT_cong_(j−1) + (1 − a)RTT_j
    Here, assume a = 0.85 which is an exponential smoothing factor. Then estimate the
    congestion level c using the following equation:
    c = min( RTT − RTT_min / RTT_cong − RTT_min, 1)
  */

  float a = 0.85;
  if(m_rtt_j1_conj < m_rtt_min) a = 0.0;    //initially, a = 0
  
  double conjRtt = a * m_rtt_j1_conj.GetSeconds() +(1-a) * m_rtt_j_loss.GetSeconds();

  m_rtt_j_conj = Seconds(conjRtt);

  NS_LOG_LOGIC("RTT_conj of j-th packet loss : " << m_rtt_j_conj << " ; RTT_conj of (j-1)st packet loss : " << m_rtt_j1_conj << " ; RTT of j-th packet loss : " << m_rtt_j_loss);

  return std::min( (m_rtt_curr.GetSeconds() - m_rtt_min.GetSeconds()) /(conjRtt - m_rtt_min.GetSeconds()), 1.0 );
}

void 
TcpAdaptiveReno::EstimateIncWnd(Ptr<TcpSocketState> tcb)
{
  /*
    W_inc_max = B/M ∗ MSS
    α = 10
    β = 2 * W_inc_max(1/α − (1/α + 1)/e^α)
    γ = 1 − 2 * W_inc_max(1/α − (1/α + 1/2)/e^α)
    W_inc(c) = W_inc_max/e^(cα) + cβ + γ
  */

  double c = EstimateCongestionLevel();
  int M = 1000; 
  
  double W_inc_max = m_currentBW.Get().GetBitRate() / M * static_cast<double>(tcb->m_segmentSize * tcb->m_segmentSize) ; 

  double alpha = 10;
  double beta = 2 * W_inc_max *((1/alpha) -((1/alpha + 1)/(std::exp(alpha))));
  double gamma = 1 -(2 * W_inc_max *((1/alpha) -((1/alpha + 0.5)/(std::exp(alpha)))));

  m_wnd_inc =(int)((W_inc_max / std::exp(c * alpha)) + c * beta + gamma);

  NS_LOG_LOGIC("Maximum window increase : " << W_inc_max << "; congestion : "<< c <<" ; beta : "<< beta <<" ; gamma : "<< gamma);
  NS_LOG_LOGIC("Increased window : "<< m_wnd_inc <<" ; previous window : "<< tcb->m_cWnd <<" ; new window : "<< (m_wnd_inc /(int)tcb->m_cWnd) );
}

void
TcpAdaptiveReno::CongestionAvoidance(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  /*
    W_base = W_base + 1MSS/W
    W_probe = max(W_probe + W_inc/W, 0)
    W = W_base + W_probe

    In the first line, W is the previous conjestion window
    while in the last one, W is the updated conjestion window
  */

  NS_LOG_FUNCTION(this << tcb << segmentsAcked);
  
  if(segmentsAcked > 0)
    {
      EstimateIncWnd(tcb);

      double base_inc = static_cast<double>(tcb->m_segmentSize * tcb->m_segmentSize) / tcb->m_cWnd.Get();
      base_inc = std::max(1.0, base_inc);
      m_wnd_base += static_cast<uint32_t>(base_inc);

      m_wnd_probe = std::max( (double)(m_wnd_probe + m_wnd_inc /(int)tcb->m_cWnd.Get()), (double) 0 );
      
      NS_LOG_LOGIC("Before "<< tcb->m_cWnd << " ; base "<< m_wnd_base <<" ; probe "<< m_wnd_probe);
      tcb->m_cWnd = m_wnd_base + m_wnd_probe;

      NS_LOG_INFO("In CongAvoid, updated to cwnd " << tcb->m_cWnd << " ssthresh " << tcb->m_ssThresh);
    }
}

} // namespace ns3