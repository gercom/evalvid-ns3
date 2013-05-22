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
#include "evalvid-client.h"
//#include "seq-ts-header.h"
#include <stdlib.h>
#include <stdio.h>
#include "ns3/string.h"

#include <iomanip>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EvalvidClient");
NS_OBJECT_ENSURE_REGISTERED (EvalvidClient);

TypeId
EvalvidClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EvalvidClient")
    .SetParent<Application> ()
    .AddConstructor<EvalvidClient> ()
    .AddAttribute (
                   "RemoteAddress",
                   "The destination Ipv4Address of the outbound packets",
                   Ipv4AddressValue (),
                   MakeIpv4AddressAccessor (&EvalvidClient::m_peerAddress),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("RemotePort", "The destination port of the outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&EvalvidClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
   .AddAttribute ("Port",
				  "Port on which we listen for incoming packets.",
				  UintegerValue (100),
				  MakeUintegerAccessor (&EvalvidClient::m_port),
				  MakeUintegerChecker<uint16_t> ())
  .AddAttribute ("RecvDumpFilename",
				  "Receive Dump Filename",
				  StringValue(""),
				  MakeStringAccessor(&EvalvidClient::rd_filename),
				  MakeStringChecker())
    ;
  return tid;
}

EvalvidClient::EvalvidClient ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

EvalvidClient::~EvalvidClient ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
EvalvidClient::SetRemote (Ipv4Address ip, uint16_t port)
{
  m_peerAddress = ip;
  m_peerPort = port;
}

void
EvalvidClient::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Application::DoDispose ();
}

void
EvalvidClient::StartApplication (void)
{


  NS_LOG_FUNCTION_NOARGS ();

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      m_socket->Bind ();
      m_socket->Connect (InetSocketAddress (m_peerAddress, m_peerPort));
    }


  rdTrace.open(rd_filename.c_str(), ios::out);
  if (rdTrace.fail())
  {
	  NS_LOG_ERROR("Error while opening output file: " << rd_filename.c_str());

  		return;
  }

  m_socket->SetRecvCallback (MakeCallback (&EvalvidClient::HandleRead, this));

  m_sendEvent = Simulator::Schedule ( Seconds(1) , &EvalvidClient::Send, this);
  m_sendEvent = Simulator::Schedule ( Seconds(2) , &EvalvidClient::Send, this);
  m_sendEvent = Simulator::Schedule ( Seconds(3) , &EvalvidClient::Send, this);
  m_sendEvent = Simulator::Schedule ( Seconds(4) , &EvalvidClient::Send, this);

}

void
EvalvidClient::Send (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  int m_size = 1;

  Ptr<Packet> p = Create<Packet> (m_size);

  //fix LTE issue, thx Louis Christodoulou
  SeqTsHeader seqTs;
  seqTs.SetSeq (0);
  p->AddHeader (seqTs);

  m_size=+ sizeof(seqTs);

  m_socket->Send (p);

  NS_LOG_INFO ("Envio de pedido : Sent " << m_size << " bytes to " << m_peerAddress);
}


void
EvalvidClient::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();
  rdTrace.close();
  if (m_socket != 0)
  {
	m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  }
  Simulator::Cancel (m_sendEvent);
}

void
EvalvidClient::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {

	  NS_LOG_INFO ("+++++ Received id :" << packet->GetUid() << " Size: " << packet->GetSize());
      if (InetSocketAddress::IsMatchingType (from))
        {
          if (packet->GetSize () > 0)
    	         {
        	  	  	  //fix LTE issue, thx Louis Christodoulou
        	  	  	  SeqTsHeader seqTs;
        	  	  	  packet->RemoveHeader (seqTs);
        	  	  	  uint32_t currentSequenceNumber = seqTs.GetSeq ();
        	  	  	  NS_LOG_INFO("Packet Sequence Number: " << currentSequenceNumber);


    		  	  	  //InetSocketAddress address = InetSocketAddress::ConvertFrom (from);
    		  	  	  Time time_now = Simulator::Now();

    		  	  	  NS_LOG_INFO("RECEBIDO " << std::fixed << std::setprecision(4) << time_now.ToDouble(ns3::Time::S)
    		          							<< std::setfill(' ') << std::setw(16) <<  "id " << currentSequenceNumber
    		          							<< std::setfill(' ') <<  std::setw(16) <<  "udp " << packet->GetSize()
    		          							<< std::endl);

    		          //rdTrace << time_now.GetSeconds() << std::setfill(' ') << std::setw(16)
    		          //	  	  <<  "id " << packet->GetUid() << std::setfill(' ') <<  std::setw(16)
    		          //	  	  <<  "udp " << packet->GetSize() << std::endl;
    		          rdTrace << std::fixed << std::setprecision(4) << time_now.ToDouble(ns3::Time::S)
    		          							<< std::setfill(' ') << std::setw(16) <<  "id " << currentSequenceNumber
    		          							<< std::setfill(' ') <<  std::setw(16) <<  "udp " << packet->GetSize()
    		          							<< std::endl;

    	         }
        }
    }
}


} // Namespace ns3
