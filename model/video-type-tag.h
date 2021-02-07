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

#ifndef VIDEO_TYPE_TAG_H
#define VIDEO_TYPE_TAG_H

#include "ns3/tag.h"

namespace ns3 {

class VideoPacketTypeTag : public Tag
 {
 public:
   VideoPacketTypeTag ();
   VideoPacketTypeTag (std::string videoPacketType);

   void SetVideoPacketType (std::string n);
   std::string GetVideoPacketType (void) const;
   static TypeId GetTypeId (void);
   virtual TypeId GetInstanceTypeId (void) const;
   virtual uint32_t GetSerializedSize (void) const;
   virtual void Serialize (TagBuffer i) const;
   virtual void Deserialize (TagBuffer i);
   virtual void Print (std::ostream &os) const;
 
 private:
   std::string m_videoPacketType; 
 };

} // namespace ns3

#endif /* VIDEO_TYPE_TAG_H */
