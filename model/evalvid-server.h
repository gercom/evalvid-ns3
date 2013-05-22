/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Billy Pinheiro <haquiticos@gmail.com>
 *
 */

#ifndef __EVALVID_SERVER_H__
#define __EVALVID_SERVER_H__

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/ipv4-address.h"
#include "ns3/seq-ts-header.h"

#include "ns3/socket.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <fstream>
#include <iomanip>

using std::ifstream;
using std::ofstream;
using std::ostream;

using std::ios;
using std::endl;



namespace ns3 {
/**
 * \ingroup applications
 * \defgroup Evalvid Evalvid
 */

/**
 * \ingroup Evalvid
 * \class EvlavidClient
 * \brief A Udp server. Receives UDP packets from a remote host. UDP packets
 * carry a 32bits sequence number followed by a 64bits time stamp in their
 * payloads. The application uses, the sequence number to determine if a packet
 * is lost, and the time stamp to compute the delay
 */
class EvalvidServer : public Application
{
public:
  static TypeId GetTypeId (void);
  EvalvidServer ();
  virtual ~EvalvidServer ();
  /**
   * returns the number of lost packets
   * \return the number of lost packets
   */
  uint32_t GetLost (void) const;

  /**
   * \brief returns the number of received packets
   * \return the number of received packets
   */
  uint32_t GetReceived (void) const;

  //evalvid structs
  struct tracerec {
		Time trec_time; /* inter-packet time (usec) */
		u_int32_t trec_size; /* frame size (bytes) */
		char trec_type; /* packet type */
		u_int32_t trec_prio; /* packet priority */
		u_int32_t trec_id; /* maximun fragmented size (bytes) */
	};
	ofstream sdTrace;

protected:
  virtual void DoDispose (void);

  unsigned int max, ndx, a_, fullPkt, restPkt, i, nrec, id;
  std::string st_filename,sd_filename;
  struct tracerec *trace;
  struct tracerec trec_;
  Time startTime,videoTime;

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void Setup (void);
  void ProcessStreamData();
  void GetNextFrame(unsigned int& ndx, struct tracerec& t);
  void HandleRead (Ptr<Socket> socket);
  void Send (int nbyte);

  ofstream rdTrace;
  uint16_t m_port;
  Ptr<Socket> m_socket;
  Address m_local;
  uint32_t m_received;
  //PacketLossCounter m_lossCounter;
  Ipv4Address m_peerAddress;
  uint16_t m_peerPort;
  EventId m_sendEvent;
  uint32_t m_count;
  Time m_interval;
  uint32_t m_size;
  uint32_t m_sent;

  Address from_client;
};

} // namespace ns3

#endif // __EVALVID_SERVER_H__
