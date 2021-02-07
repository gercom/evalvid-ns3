/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
 * Copyright (c) 2021 IWING, Department of Computer Engineering, Kasetsart University 
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *         Aphirak JANSANG <aphirak.j@ku.ac.th>
 */
#include "video-type-tag.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("VideoPacketTypeTag");


/***************************************************************
 *           VideoPacketType Tags
 ***************************************************************/

VideoPacketTypeTag::VideoPacketTypeTag ()
  :Tag()
{
  NS_LOG_FUNCTION (this);
}

VideoPacketTypeTag::VideoPacketTypeTag (std::string videoPacketType)
  :Tag(), m_videoPacketType(videoPacketType)
{
  NS_LOG_FUNCTION (this << videoPacketType);
}

void
VideoPacketTypeTag::SetVideoPacketType (std::string n)
{
  NS_LOG_FUNCTION (this << n);
  m_videoPacketType = n;
}

std::string
VideoPacketTypeTag::GetVideoPacketType (void) const
{
  NS_LOG_FUNCTION (this);
  return m_videoPacketType;
}

NS_OBJECT_ENSURE_REGISTERED (VideoPacketTypeTag);

TypeId
VideoPacketTypeTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::VideoPacketTypeTag")
    .SetParent<Tag> ()
    .SetGroupName("Network")
    .AddConstructor<VideoPacketTypeTag> ();
  return tid;
}
TypeId
VideoPacketTypeTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t
VideoPacketTypeTag::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  uint32_t s = 1 + m_videoPacketType.size();  // +1 for name length field
  return s;
}
void
VideoPacketTypeTag::Serialize (TagBuffer i) const
{
  NS_LOG_FUNCTION (this << &i);
  const char *n = m_videoPacketType.c_str();
  uint8_t l = (uint8_t) m_videoPacketType.size ();

  i.WriteU8 (l);
  i.Write ( (uint8_t*) n , (uint32_t) l);
}
void
VideoPacketTypeTag::Deserialize (TagBuffer i)
{
  NS_LOG_FUNCTION (this << &i);
  uint8_t l = i.ReadU8();
  char buf[256];

  i.Read ( (uint8_t* ) buf, (uint32_t) l);
  m_videoPacketType = std::string (buf, l);
}
void
VideoPacketTypeTag::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "VideoPacketType=" << m_videoPacketType;
}

} // namespace ns3

