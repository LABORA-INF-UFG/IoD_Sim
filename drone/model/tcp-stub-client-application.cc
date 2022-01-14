/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2021 The IoD_Sim Authors.
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
 */
#include <ns3/drone.h>
#include <ns3/seq-ts-header.h>
#include <ns3/simulator.h>
#include <ns3/socket-factory.h>
#include <ns3/tcp-socket-factory.h>
#include <ns3/uinteger.h>
#include <ns3/nstime.h>

#include "tcp-stub-client-application.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpStubClientApplication");
NS_OBJECT_ENSURE_REGISTERED (TcpStubClientApplication);

TypeId
TcpStubClientApplication::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::TcpStubClientApplication")
    .SetParent<TcpStorageClientApplication> ()
    .SetGroupName ("Applications")
    .AddConstructor<TcpStubClientApplication> ()
    .AddAttribute ("TxFrequency", "Transmission frequency for packets.",
                   DoubleValue (1),
                   MakeDoubleAccessor (&TcpStubClientApplication::m_txFrequency),
                   MakeDoubleChecker<double> (0))
    ;

  return tid;
}

TcpStubClientApplication::TcpStubClientApplication ()
{
  NS_LOG_FUNCTION (this);
}

TcpStubClientApplication::~TcpStubClientApplication ()
{
  NS_LOG_FUNCTION (this);
}

void
TcpStubClientApplication::DoInitialize ()
{
  m_txInterval = Seconds (1.0 / m_txFrequency);
}

void
TcpStubClientApplication::StartApplication ()
{
  NS_LOG_FUNCTION (this);
  TcpClientServerApplication::StartApplication ();
  Connect ();

  Simulator::Schedule (m_txInterval, &TcpStubClientApplication::SendPacket, this);
}

void
TcpStubClientApplication::SendPacket ()
{
  NS_LOG_FUNCTION (this << GetNode ()->GetId ());
  DoSendPacket (GetPayloadSize ());
  Simulator::Schedule (m_txInterval, &TcpStubClientApplication::SendPacket, this);
}

} // namespace ns3
