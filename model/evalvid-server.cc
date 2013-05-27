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

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
//#include "packet-loss-counter.h"

#include "evalvid-server.h"


#include <iomanip>

using std::ifstream;
using std::ofstream;
using std::ostream;

using std::ios;
using std::endl;


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EvalvidServer");
NS_OBJECT_ENSURE_REGISTERED (EvalvidServer);


TypeId
EvalvidServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EvalvidServer")
    .SetParent<Application> ()
    .AddConstructor<EvalvidServer> ()
    .AddAttribute ("Port",
                   "Port on which we listen for incoming packets.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&EvalvidServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
 	   .AddAttribute (
			   "RemoteAddress",
			   "The destination Ipv4Address of the outbound packets",
			   Ipv4AddressValue (),
			   MakeIpv4AddressAccessor (&EvalvidServer::m_peerAddress),
			   MakeIpv4AddressChecker ())
	   .AddAttribute ("RemotePort", "The destination port of the outbound packets",
			   UintegerValue (100),
			   MakeUintegerAccessor (&EvalvidServer::m_peerPort),
			   MakeUintegerChecker<uint16_t> ())
	   .AddAttribute ("PacketSize",
			   "Size of packets generated. The minimum packet size is 12 bytes which is the size of the header carrying the sequence number and the time stamp.",
			   UintegerValue (1024),
			   MakeUintegerAccessor (&EvalvidServer::m_size),
			   MakeUintegerChecker<uint32_t> (12,1500))
	   .AddAttribute ("MaxPackets",
			   "The maximum number of packets the application will send",
			   UintegerValue (100),
			   MakeUintegerAccessor (&EvalvidServer::max),
			   MakeUintegerChecker<uint32_t> ())
	   .AddAttribute ("Interval",
			   "The time to wait between packets", TimeValue (Seconds (1.0)),
			   MakeTimeAccessor (&EvalvidServer::m_interval),
			   MakeTimeChecker ())
	   .AddAttribute ("SendDumpFilename",
			   "Send Dump Filename",
			   StringValue(""),
			   MakeStringAccessor(&EvalvidServer::sd_filename),
			   MakeStringChecker())
	   .AddAttribute ("SendTraceFilename",
			   "Send trace Filename",
			   StringValue(""),
			   MakeStringAccessor(&EvalvidServer::st_filename),
			   MakeStringChecker())
    ;
  return tid;
}

EvalvidServer::EvalvidServer ()
 {
  NS_LOG_FUNCTION (this);
  m_received=0;
  m_sendEvent = EventId ();
}

EvalvidServer::~EvalvidServer ()
{
  NS_LOG_FUNCTION (this);
}

uint32_t
EvalvidServer::GetReceived (void) const
{

  return m_received;

}

void
EvalvidServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
EvalvidServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  max = m_size;

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (),
                                                   m_port);
      m_socket->Bind (local);
    }

   ndx = 0;
    nrec = 0;
    id = 0;
    a_ = 0;

   startTime = Now();
   videoTime = Seconds(0);


    Setup();
    GetNextFrame(ndx, trec_);
    startTime = trec_.trec_time;

    m_socket->SetRecvCallback(MakeCallback(&EvalvidServer::HandleRead, this));
}

void
EvalvidServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);
  Simulator::Cancel (m_sendEvent);
}

void
EvalvidServer::Setup()
{
	unsigned long id, size, prio;
	double vtime;
	char type;
	tracerec* t;

	sdTrace.open(sd_filename.c_str(), ios::out);
	if (sdTrace.fail()) {
		NS_LOG_ERROR("Error while opening output file: " << sd_filename.c_str());
		return;
	}

	//ifstream traceFile(filename, ios::in);
	ifstream traceFile(st_filename.c_str(), ios::in);

	if (traceFile.fail()) {

		NS_LOG_ERROR("Error while opening input file: " << st_filename.c_str());
		return;
	}
	while (traceFile >> id >> type >> size >> prio >> vtime) {
		nrec++;
	}

	NS_LOG_INFO("Register Numbers: " << nrec);

	traceFile.clear();
	traceFile.seekg(0,ios::beg);
	trace = new struct tracerec[nrec];
	t = trace;

	while (traceFile >> id >> type >> size >> prio >> vtime) {
		t->trec_id = id;
		t->trec_type = type;
		t->trec_size = size;
		t->trec_prio = prio;
		t->trec_time = Seconds(vtime);
		NS_LOG_INFO(id << " "<<type <<" "<< size<<" " << prio <<" "<< t->trec_time.GetSeconds() );
		t++;

	}
}

void
EvalvidServer::GetNextFrame(unsigned int& ndx, struct tracerec& t)
{
	t.trec_time = trace[ndx].trec_time;
	t.trec_size = trace[ndx].trec_size;
	t.trec_type = trace[ndx].trec_type;
	t.trec_prio = trace[ndx].trec_prio;
	t.trec_id = trace[ndx].trec_id;

	//i dont know why but was not working inside the IF(bug 1, fixed)
	ndx++;
	if (ndx == nrec) {
		NS_LOG_INFO("######################################################## " << t.trec_id );
		ndx = 0;
		a_ = 1;
	}
}

void
EvalvidServer::ProcessStreamData()
{

	fullPkt = trec_.trec_size / max;
	restPkt = trec_.trec_size % max;

	NS_LOG_INFO("id pkt: " <<  trec_.trec_id << " trec_size :" << trec_.trec_size );
	NS_LOG_INFO("fullpkt :" << fullPkt << " restPkt: " << restPkt);

	if (fullPkt > 0) {
		for (i = 0; i < fullPkt; i++) {
			Send(max);
		}
	}
	if (restPkt != 0) {
		Send(restPkt);
	}
    

	Time interval = trec_.trec_time;

	GetNextFrame(ndx, trec_);

	Time vtime = trec_.trec_time - interval ;
	
	videoTime += vtime;

	NS_LOG_INFO("pacotes ID : " << trec_.trec_id << " no tempo: " << (Simulator::Now() + vtime).GetSeconds() << " Agora : " << Simulator::Now().GetSeconds() << " diferenca: " << vtime.GetSeconds() << " Tempo Video: " << videoTime.GetSeconds()  );

	//if there is frames to send, so schedule that
	if(a_ == 0)
	{

		m_sendEvent = Simulator::Schedule (vtime, &EvalvidServer::ProcessStreamData, this);
	}
}


void
EvalvidServer::Send (int nbytes)
{
  NS_LOG_FUNCTION_NOARGS ();
  //i comment that, i dont really now what its does, but was not working with.
  //NS_ASSERT (m_sendEvent.IsExpired ());
  Ptr<Packet> p = Create<Packet> (nbytes );


  NS_LOG_INFO("Size after: " << p->GetSize());

  Time time_now = Simulator::Now();
  sdTrace << std::fixed << std::setprecision(4) << time_now.ToDouble(Time::S)
  		  << std::setfill(' ') << std::setw(16) <<  "id " << m_sent
  		  << std::setfill(' ') <<  std::setw(16) <<  "udp " << p->GetSize()
  		  << std::endl;

  //fix LTE issue, thx Louis Christodoulou
    NS_LOG_INFO("Size before: " << p->GetSize());
    SeqTsHeader seqTs;
    seqTs.SetSeq (m_sent);
    p->AddHeader (seqTs);

  NS_LOG_INFO("IPPPPPPPPPPP do Cliente: " << from_client);

  if ((m_socket->SendTo(p,0, from_client )) >= 0)
    {
      ++m_sent;
     NS_LOG_LOGIC("#### TraceDelay TX " << m_size << " bytes to "
                  << m_peerAddress << " Uid: " << m_sent
                  << " Time: " << (Simulator::Now ()).GetSeconds ());

    }
  else
    {
      NS_LOG_INFO ("Error while sending " << m_size << " bytes to "
                   << m_peerAddress);
    }
}

void
EvalvidServer::HandleRead (Ptr<Socket> socket)
{
	NS_LOG_LOGIC("\n perdido de inicio de transmissão no server \n");
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (InetSocketAddress::IsMatchingType (from))
        {
    	  //bug fix

          InetSocketAddress address = InetSocketAddress::ConvertFrom (from);
          m_peerAddress = address.GetIpv4();
          NS_LOG_INFO ("Received " << packet->GetSize() << " bytes from " <<
            address.GetIpv4());

          packet->RemoveAllPacketTags ();
          packet->RemoveAllByteTags ();

          Ptr<Packet> p = Create<Packet> (0);
          if ((m_socket->SendTo(p,0, from )) >= 0)
        	  NS_LOG_INFO("First packet to client sent");
          else
          	  NS_LOG_INFO("Error sending first packet");

          from_client = from;
          NS_LOG_LOGIC ("Sending video");
          m_sendEvent = Simulator::Schedule (startTime,  &EvalvidServer::ProcessStreamData, this);
        }
    }
}

} // Namespace ns3
