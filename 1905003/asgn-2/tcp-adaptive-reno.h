#ifndef TCP_ADAPTIVERENO_H
#define TCP_ADAPTIVERENO_H

#include "tcp-westwood-plus.h"

namespace ns3 {

class Time;

/**
 * \ingroup congestionOps
 *
 * \brief An implementation of TCP Adaptive Reno
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
  double EstimateCongestionLevel();
  void EstimateIncWnd(Ptr<TcpSocketState> tcb);

protected:
  void CongestionAvoidance (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked) override;

  Time                   m_rtt_min;               //!< Minimum RTT
  Time                   m_rtt_curr;              //!< Current RTT
  Time                   m_rtt_j_loss;            //!< RTT of j-th packet loss
  Time                   m_rtt_j_conj;            //!< RTT_conj of j-th packet loss
  Time                   m_rtt_j1_conj;           //!< RTT_conj of (j-1)st packet loss
  int32_t                m_wnd_inc;               //!< Increment Window
  uint32_t               m_wnd_base;              //!< Base Window
  int32_t                m_wnd_probe;             //!< Probe Window 
};

}

#endif /* TCP_ADAPTIVE_RENO_H */