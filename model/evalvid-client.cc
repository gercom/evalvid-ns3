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
 *         Saulo da Mata <damata.saulo@gmail.com>
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
#include "video-type-tag.h"

#include <stdlib.h>
#include <stdio.h>
#include "ns3/string.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EvalvidClient");
NS_OBJECT_ENSURE_REGISTERED (EvalvidClient);

TypeId
EvalvidClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EvalvidClient")
    .SetParent<Application> ()
    .AddConstructor<EvalvidClient> ()
    .AddAttribute ("RemoteAddress",
                   "The destination Ipv4Address of the outbound packets",
                   Ipv4AddressValue (),
                   MakeIpv4AddressAccessor (&EvalvidClient::m_peerAddress),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("RemotePort", "The destination port of the outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&EvalvidClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("ReceiverDumpFilename",
                   "Receiver Dump Filename",
                   StringValue(""),
                   MakeStringAccessor(&EvalvidClient::receiverDumpFileName),
                   MakeStringChecker())
    .AddAttribute ("MutlicastGroupAddress",
                   "The multicast group address for video traffic",
                   Ipv4AddressValue (),
                   MakeIpv4AddressAccessor (&EvalvidClient::m_peerMcastIpv4Address),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("EnableRequest",
                   "Enable the client to send the request to evalvid server (default: enable)",
                   BooleanValue (true),
                   MakeBooleanAccessor (&EvalvidClient::m_enableRequest),
                   MakeBooleanChecker ())
    ;
  return tid;
}

EvalvidClient::EvalvidClient ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_sendEvent = EventId ();
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
  NS_LOG_FUNCTION_NOARGS();

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      m_socket->Bind ();
      m_socket->Connect (InetSocketAddress (m_peerAddress, m_peerPort));

      //Multicast socket
      //peer: mcast group -- ipv4
      if (m_peerMcastIpv4Address.IsInitialized()) {
        m_peerMcastAddress = Address(InetSocketAddress(m_peerMcastIpv4Address, m_peerPort));
        m_socketMcast = Socket::CreateSocket (GetNode (), tid);
        if (m_socketMcast->Bind (m_peerMcastAddress) == -1)
          {
            NS_FATAL_ERROR ("Failed to bind socket");
          }
        m_socketMcast->Bind ();
        m_socketMcast->Listen ();
        m_socketMcast->ShutdownSend ();
        if (addressUtils::IsMulticast (m_peerMcastAddress))
          {
            Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socketMcast);
            if (udpSocket)
              {
                // equivalent to setsockopt (MCAST_JOIN_GROUP)
                udpSocket->MulticastJoinGroup (0, m_peerMcastAddress);
              }
            else
              {
                NS_FATAL_ERROR ("Error: joining multicast on a non-UDP socket");
              }
          }
        }
    }

  receiverDumpFile.open(receiverDumpFileName.c_str(), ios::out);
  if (receiverDumpFile.fail())
    {
      NS_FATAL_ERROR(">> EvalvidClient: Error while opening output file: " << receiverDumpFileName.c_str());
      return;
    }

  m_socket->SetRecvCallback (MakeCallback (&EvalvidClient::HandleRead, this));
  //for receiving multicast video traffic
  if (m_peerMcastIpv4Address.IsInitialized()) {
    m_socketMcast->SetRecvCallback (MakeCallback (&EvalvidClient::HandleRead, this));
  }

  //Delay requesting to get server on line.
  if (m_enableRequest) {
    m_sendEvent = Simulator::Schedule ( Seconds(0.1) , &EvalvidClient::Send, this);
  }


}

void
EvalvidClient::Send (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  Ptr<Packet> p = Create<Packet> ();

  SeqTsHeader seqTs;
  seqTs.SetSeq (0);
  p->AddHeader (seqTs);

  m_socket->Send (p);

  NS_LOG_INFO (">> EvalvidClient: Sending request for video streaming to EvalvidServer at "
                << m_peerAddress << ":" << m_peerPort);
}


void
EvalvidClient::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();
  receiverDumpFile.close();
  Simulator::Cancel (m_sendEvent);
}

void
EvalvidClient::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  Address localAddress;
  while ((packet = socket->RecvFrom (from)))
    {
      if (InetSocketAddress::IsMatchingType (from))
        {
          if (packet->GetSize () > 0)
            {
              SeqTsHeader seqTs;
              packet->RemoveHeader (seqTs);
              uint32_t packetId = seqTs.GetSeq ();

              // remove the VideoPacketTypeTag from received packet -- evalvid
              VideoPacketTypeTag vTag;
              bool res = packet->RemovePacketTag (vTag);

              NS_LOG_DEBUG(">> EvalvidClient: Received packet at " << Simulator::Now().GetSeconds()
                           << "s\tid: " << packetId
                           << "\tudp\t" << packet->GetSize() << 
                           "\tfrom\t" << InetSocketAddress::ConvertFrom (from).GetIpv4 () <<std::endl); 

              receiverDumpFile << std::fixed << std::setprecision(4) << Simulator::Now().ToDouble(ns3::Time::S)
                               << std::setfill(' ') << std::setw(16) <<  "id " << packetId
                               << std::setfill(' ') <<  std::setw(16) <<  "udp " << packet->GetSize()
                               << std::endl;
           }
        }
    }
}

} // Namespace ns3
