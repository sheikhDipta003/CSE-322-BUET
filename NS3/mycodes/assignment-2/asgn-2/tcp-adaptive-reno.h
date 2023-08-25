#ifndef TCP_ADAPTIVERENO_H
#define TCP_ADAPTIVERENO_H

#include "tcp-congestion-ops.h"
#include "tcp-westwood-plus.h"
#include "ns3/tcp-recovery-ops.h"
#include "ns3/sequence-number.h"
#include "ns3/traced-value.h"
#include "ns3/event-id.h"

namespace ns3 {

class Packet;
class TcpHeader;
class Time;
class EventId;

/**
 * \ingroup congestionOps
 *
 * \brief An implementation of TCP ADAPTIVE RENO.
 *
 */
class TcpAdaptiveReno : public TcpWestwoodPlus
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  TcpAdaptiveReno (void);
  /**
   * \brief Copy constructor
   * \param sock the object to copy
   */
  TcpAdaptiveReno (const TcpAdaptiveReno& sock);
  ~TcpAdaptiveReno (void) override;

  /**
   * \brief Filter type (None or Tustin)
   */
  enum FilterType 
  {
    NONE,
    TUSTIN
  };

  uint32_t GetSsThresh (Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight) override;

  void PktsAcked (Ptr<TcpSocketState> tcb, uint32_t packetsAcked, const Time& rtt) override;

  Ptr<TcpCongestionOps> Fork () override;

private:
  void EstimateBW (const Time& rtt, Ptr<TcpSocketState> tcb);

  double EstimateCongestionLevel();

  void EstimateIncWnd(Ptr<TcpSocketState> tcb);

protected:
  void CongestionAvoidance (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked) override;

  Time                   m_minRtt;                 //!< Minimum RTT
  Time                   m_currentRtt;             //!< Current RTT
  Time                   m_jPacketLRtt;            //!< RTT of j packet loss
  Time                   m_conjRtt;                //!< Conjestion RTT (j th event)
  Time                   m_prevConjRtt;            //!< Previous Conjestion RTT (j-1 th event)

  // Window calculations
  int32_t                m_incWnd;                 //!< Increment Window
  uint32_t               m_baseWnd;                //!< Base Window
  int32_t                m_probeWnd;               //!< Probe Window 
};

}

#endif /* TCP_ADAPTIVE_RENO_H */