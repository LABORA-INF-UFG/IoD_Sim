/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2022 The IoD_Sim Authors.
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
// Standard Library
#include <vector>
#include <algorithm>
#include <thread>
#include <pthread.h>
#include <iostream>
#include <sstream>

// NS3 Core Components
#include <ns3/buildings-helper.h>
#include <ns3/config.h>
#include <ns3/csma-module.h>
#include <ns3/internet-module.h>
#include <ns3/ipv4-address-helper.h>
#include <ns3/ipv4-static-routing-helper.h>
#include <ns3/ipv4-routing-helper.h>
#include <ns3/log.h>
#include <ns3/lte-enb-net-device.h>
#include <ns3/lte-ue-net-device.h>
#include <ns3/mobility-helper.h>
#include <ns3/object-factory.h>
#include <ns3/net-device-container.h>
#include <ns3/ptr.h>
#include <ns3/show-progress.h>
#include <ns3/ssid.h>
#include <ns3/string.h>
// IoD Sim Report Module
#include <ns3/report.h>
// IoD Sim Components
#include <ns3/drone-client-application.h>
#include <ns3/drone-container.h>
#include <ns3/drone-energy-model-helper.h>
#include <ns3/drone-list.h>
#include <ns3/drone-peripheral.h>
#include <ns3/drone-peripheral-container.h>
#include <ns3/drone-server-application.h>
#include <ns3/energy-source.h>
#include <ns3/input-peripheral.h>
#include <ns3/ipv4-network-layer-configuration.h>
#include <ns3/ipv4-simulation-helper.h>
#include <ns3/lte-netdevice-configuration.h>
#include <ns3/lte-phy-layer-configuration.h>
#include <ns3/lte-phy-simulation-helper.h>
#include <ns3/lte-setup-helper.h>
#include <ns3/mobility-factory-helper.h>
#include <ns3/nat-application.h>
#include <ns3/scenario-configuration-helper.h>
#include <ns3/wifi-netdevice-configuration.h>
#include <ns3/wifi-mac-factory-helper.h>
#include <ns3/wifi-mac-layer-configuration.h>
#include <ns3/wifi-mac-simulation-helper.h>
#include <ns3/wifi-phy-factory-helper.h>
#include <ns3/wifi-phy-layer-configuration.h>
#include <ns3/wifi-phy-simulation-helper.h>
#include <ns3/zsp-list.h>
#include <ns3/remote-list.h>
#include <ns3/node-list.h>
#include <ns3/traced-value.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/interest-region-container.h>
#include <ns3/debug-helper.h>
// Flow Monitor 1
#include <ns3/flow-monitor-module.h>
// Crow module
#include "crow.h"
// RapidJSON
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include <rapidjson/writer.h>
// Modern json
#include <nlohmann/json.hpp>

//
// context
class ScenarioContext;

namespace ns3 {

constexpr int N_LAYERS = 4;
constexpr int PHY_LAYER = 0;
constexpr int MAC_LAYER = 1;
constexpr int NET_LAYER = 2;
constexpr int APP_LAYER = 3;
constexpr int PROGRESS_REFRESH_INTERVAL_SECONDS = 1;

class Scenario
{
public:
  Scenario (std::string req_body); //, ScenarioContext *sc);
  virtual ~Scenario ();
  // void startSimulation (std::string req_body, ScenarioContext *sc);

  void operator() ();

private:
  void ApplyStaticConfig ();
  void ConfigureWorld ();
  void ConfigurePhy ();
  void ConfigureMac ();
  void ConfigureNetwork ();
  void EnableIpv4RoutingTableReporting ();
  void ConfigureEntities (const std::string &entityKey, NodeContainer &nodes);
  void ConfigureEntityMobility (const std::string &entityKey, Ptr<EntityConfiguration> entityConf,
                                const uint32_t entityId);
  NetDeviceContainer ConfigureEntityWifiStack (std::string entityKey,
                                               Ptr<NetdeviceConfiguration> entityNetDev,
                                               Ptr<Node> entityNode, const uint32_t entityId,
                                               const uint32_t deviceId, const uint32_t netId);
  void ConfigureLteEnb (Ptr<Node> entityNode, const uint32_t netId);
  void ConfigureLteUe (Ptr<Node> entityNode, const std::vector<LteBearerConfiguration> bearers,
                       const uint32_t netId);
  void InstallEntityIpv4 (Ptr<Node> entityNode, NetDeviceContainer netDevices,
                          const uint32_t netId);
  void InstallEntityIpv4 (Ptr<Node> entityNode, Ptr<NetDevice> netDevice, const uint32_t netId);
  void ConfigureEntityIpv4 (Ptr<Node> entityNode, NetDeviceContainer devContainer,
                            const uint32_t deviceId, const uint32_t netId);
  void ConfigureEntityApplications (const std::string &entityKey,
                                    const Ptr<EntityConfiguration> &conf, const uint32_t &entityId);
  void ConfigureEntityMechanics (const std::string &entityKey, Ptr<EntityConfiguration> entityConf,
                                 const uint32_t entityId);
  Ptr<EnergySource> ConfigureEntityBattery (const std::string &entityKey,
                                            Ptr<EntityConfiguration> entityConf,
                                            const uint32_t entityId);
  void ConfigureEntityPeripherals (const std::string &entityKey,
                                   const Ptr<EntityConfiguration> &conf, const uint32_t &entityId);
  void ConfigureInternetRemotes ();
  void ConfigureInternetBackbone ();
  void EnablePhyLteTraces ();
  void ConfigureRegionsOfInterest ();
  void CourseChange (std::string context, Ptr<const MobilityModel> model);
  void ConfigureSimulator ();
  void MetricsHandler (nlohmann::json *jout, nlohmann::json *nodes_ip,
                       ns3::FlowMonitorHelper *flowmon, Ptr<ns3::FlowMonitor> *monitor,
                       double time);
  DroneContainer m_drones;
  NodeContainer m_zsps;
  NodeContainer m_remoteNodes;
  NodeContainer m_backbone;

  std::array<std::vector<Ptr<Object>>, N_LAYERS> m_protocolStacks;
};

NS_LOG_COMPONENT_DEFINE ("Scenario");

Scenario::Scenario (std::string req_body) //, ScenarioContext *sc)
{
  // (*sc).status = "Running";
  std::cout << "Running" << std::endl;
  CONFIGURATOR->Initialize (req_body);
  m_drones.Create (CONFIGURATOR->GetDronesN ());
  m_zsps.Create (CONFIGURATOR->GetZspsN ());
  m_remoteNodes.Create (CONFIGURATOR->GetRemotesN ());
  m_backbone.Add (m_remoteNodes);

  // Register created entities in their lists
  for (auto drone = m_drones.Begin (); drone != m_drones.End (); drone++)
    DroneList::Add (*drone);

  for (auto zsp = m_zsps.Begin (); zsp != m_zsps.End (); zsp++)
    ZspList::Add (*zsp);

  for (auto remote = m_remoteNodes.Begin (); remote != m_remoteNodes.End (); remote++)
    RemoteList::Add (*remote);

  ApplyStaticConfig ();
  ConfigureWorld ();
  ConfigurePhy ();
  ConfigureMac ();
  ConfigureNetwork ();
  ConfigureRegionsOfInterest ();
  ConfigureEntities ("drones", m_drones);
  ConfigureEntities ("ZSPs", m_zsps);
  ConfigureInternetBackbone ();
  ConfigureInternetRemotes ();
  EnablePhyLteTraces ();

  //DebugHelper::ProbeNodes();
  ConfigureSimulator ();
  // (*sc).status = "Done";
  std::cout << "Done." << std::endl;
}

Scenario::~Scenario ()
{
}

//
void
Scenario::MetricsHandler (nlohmann::json *jout, nlohmann::json *nodes_ip,
                          ns3::FlowMonitorHelper *flowmon, Ptr<ns3::FlowMonitor> *monitor,
                          double time)
{
  // TODO: refatorar
  // std::uint32_t SentPackets = 0;
  // std::uint32_t ReceivedPackets = 0;
  // std::uint32_t LostPackets = 0;
  // int j = 0;
  // float AvgThroughput = 0;
  // Time Jitter;
  // Time Delay;

  // std::cout << "=========== TIME: " << time << " ===========" << std::endl;

  Ptr<Ipv4FlowClassifier> classifier =
      DynamicCast<Ipv4FlowClassifier> ((*flowmon).GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = (*monitor)->GetFlowStats ();

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin ();
       iter != stats.end (); ++iter)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);
      // auto droneId = iter->first->GetObject<Drone> ()->GetId ();
      // std::cout << "----Flow ID: " << iter->first << std::endl;
      // std::cout << "Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress
      //           << std::endl;
      // std::cout << "Sent Packets = " << iter->second.txPackets << std::endl;
      // std::cout << "Received Packets = " << iter->second.rxPackets << std::endl;
      // std::cout << "Lost Packets = " << iter->second.txPackets - iter->second.rxPackets
      //           << std::endl;
      // std::cout << "Packet delivery ratio = "
      //           << iter->second.rxPackets * 100 / iter->second.txPackets << "%" << std::endl;
      // std::cout << "Packet loss ratio = "
      //           << (iter->second.txPackets - iter->second.rxPackets) * 100 / iter->second.txPackets
      //           << "%" << std::endl;
      // std::cout << "Delay = " << iter->second.delaySum << std::endl;
      // std::cout << "Jitter = " << iter->second.jitterSum << std::endl;
      // std::cout << "Throughput = "
      //           << iter->second.rxBytes * 8.0 /
      //                  (iter->second.timeLastRxPacket.GetSeconds () -
      //                   iter->second.timeFirstTxPacket.GetSeconds ()) /
      //                  1024
      //           << "Kbps" << std::endl;

      // add to json
      std::stringstream srcAddr, dstAddr, sentPck, recvPck, lostPck, pckDelRatio, delay, jitter,
          thrgKbps, communication;
      std::string commStr;
      srcAddr << t.sourceAddress;
      dstAddr << t.destinationAddress;
      sentPck << iter->second.txPackets;
      recvPck << iter->second.rxPackets;
      lostPck << iter->second.txPackets - iter->second.rxPackets;
      pckDelRatio << (iter->second.txPackets - iter->second.rxPackets) * 100 /
                         iter->second.txPackets;
      delay << iter->second.delaySum;
      jitter << iter->second.jitterSum;
      thrgKbps << iter->second.rxBytes * 8.0 /
                      (iter->second.timeLastRxPacket.GetSeconds () -
                       iter->second.timeFirstTxPacket.GetSeconds ()) /
                      1024;
      // check if exists
      if ((*nodes_ip).count (srcAddr.str ()) == 0)
        {
          commStr = "null";
        }
      else
        {
          communication << (*nodes_ip)[srcAddr.str ()]["label"] << "->"
                        << (*nodes_ip)[dstAddr.str ()]["label"];
          commStr = communication.str ();
          commStr.erase (std::remove (commStr.begin (), commStr.end (), '\"'), commStr.end ());
          // check type
          if ((*nodes_ip)[srcAddr.str ()]["type"] == "drone") // drone
            {
              auto drone = m_drones.Get ((*nodes_ip)[srcAddr.str ()]["cIndex"]);
              auto dronePosition = drone->GetObject<MobilityModel> ()->GetPosition ();
              (*jout)["drones-metrics"].push_back (nlohmann::json::object (
                  {{"time", time},
                   {"node-id", drone->GetId ()},
                   {"battery", drone->GetObject<EnergySource> ()->GetRemainingEnergy ()},
                   {"location",
                    {{"x", dronePosition.x}, {"y", dronePosition.y}, {"z", dronePosition.z}}},
                   {"network",
                    {{"flow-monitor-id", iter->first},
                     {"iface", (*nodes_ip)[srcAddr.str ()]["iface"]},
                     {"communication", commStr},
                     {"src-addr", srcAddr.str ()},
                     {"dst-addr", dstAddr.str ()},
                     {"sent-pck", sentPck.str ()},
                     {"received-pck", recvPck.str ()},
                     {"lost-pck", lostPck.str ()},
                     {"pck-delivery-ratio", pckDelRatio.str ()},
                     {"delay", delay.str ()},
                     {"jitter", jitter.str ()},
                     {"throughput-kbps", thrgKbps.str ()},
                     {"rsrp", "TODO"},
                     {"rsrq", "TODO"},
                     {"sinr", "TODO"},
                     {"cqi", "TODO"},
                     {"txPower", "TODO"}}}}));
            }
          else if ((*nodes_ip)[srcAddr.str ()]["type"] == "zsp") // zsp
            {
              auto zsp = m_zsps.Get ((*nodes_ip)[srcAddr.str ()]["cIndex"]);
              (*jout)["zsps-metrics"].push_back (
                  nlohmann::json::object ({{"time", time},
                                           {"id", zsp->GetId ()},
                                           {"network",
                                            {{"flow-monitor-id", iter->first},
                                             {"iface", (*nodes_ip)[srcAddr.str ()]["iface"]},
                                             {"communication", commStr},
                                             {"src-addr", srcAddr.str ()},
                                             {"dst-addr", dstAddr.str ()},
                                             {"sent-pck", sentPck.str ()},
                                             {"received-pck", recvPck.str ()},
                                             {"lost-pck", lostPck.str ()},
                                             {"pck-delivery-ratio", pckDelRatio.str ()},
                                             {"delay", delay.str ()},
                                             {"jitter", jitter.str ()},
                                             {"throughput-kbps", thrgKbps.str ()},
                                             {"txPower", "TODO"}}}}));
            }
          else
            {
              std::stringstream jlabel;
              std::string jlabelStr;
              jlabel << (*nodes_ip)[srcAddr.str ()]["type"] << "s-metrics";
              jlabelStr = jlabel.str();
              jlabelStr.erase (std::remove (jlabelStr.begin (), jlabelStr.end (), '\"'), jlabelStr.end ());
              (*jout)[jlabelStr].push_back (
                  nlohmann::json::object ({{"time", time},
                                           {"node-id", (*nodes_ip)[srcAddr.str ()]["id"]},
                                           {"network",
                                            {{"flow-monitor-id", iter->first},
                                             {"iface", (*nodes_ip)[srcAddr.str ()]["iface"]},
                                             {"communication", commStr},
                                             {"src-addr", srcAddr.str ()},
                                             {"dst-addr", dstAddr.str ()},
                                             {"sent-pck", sentPck.str ()},
                                             {"received-pck", recvPck.str ()},
                                             {"lost-pck", lostPck.str ()},
                                             {"pck-delivery-ratio", pckDelRatio.str ()},
                                             {"delay", delay.str ()},
                                             {"jitter", jitter.str ()},
                                             {"throughput-kbps", thrgKbps.str ()}}}}));
            }
        }

      (*jout)["flow-metrics"].push_back (
          nlohmann::json::object ({{"time", time},
                                   // {"flow-metrics",
                                   {"id", iter->first},
                                   {"communication", commStr},
                                   {"src-addr", srcAddr.str ()},
                                   {"dst-addr", dstAddr.str ()},
                                   {"sent-pck", sentPck.str ()},
                                   {"received-pck", recvPck.str ()},
                                   {"lost-pck", lostPck.str ()},
                                   {"pck-delivery-ratio", pckDelRatio.str ()},
                                   {"delay", delay.str ()},
                                   {"jitter", jitter.str ()},
                                   {"throughput-kbps", thrgKbps.str ()}}));

      // sum to total
      // SentPackets = SentPackets + (iter->second.txPackets);
      // ReceivedPackets = ReceivedPackets + (iter->second.rxPackets);
      // LostPackets = LostPackets + (iter->second.txPackets - iter->second.rxPackets);
      // AvgThroughput = AvgThroughput + (iter->second.rxBytes * 8.0 /
      //                                  (iter->second.timeLastRxPacket.GetSeconds () -
      //                                   iter->second.timeFirstTxPacket.GetSeconds ()) /
      //                                  1024);
      // Delay = Delay + (iter->second.delaySum);
      // Jitter = Jitter + (iter->second.jitterSum);

      // j = j + 1;
    }

  // AvgThroughput = AvgThroughput / j;
  // std::cout << "--------Total Results of the simulation----------" << std::endl;
  // std::cout << "Total sent packets  = " << SentPackets << std::endl;
  // std::cout << "Total Received Packets = " << ReceivedPackets << std::endl;
  // std::cout << "Total Lost Packets = " << LostPackets << std::endl;
  // std::cout << "Packet Loss ratio = " << ((LostPackets * 100) / SentPackets) << "%" << std::endl;
  // std::cout << "Packet delivery ratio = " << ((ReceivedPackets * 100) / SentPackets) << "%"
  //           << std::endl;
  // std::cout << "Average Throughput = " << AvgThroughput << " Kbps" << std::endl;
  // std::cout << "End to End Delay = " << Delay << std::endl;
  // std::cout << "End to End Jitter delay = " << Jitter << std::endl;
  // std::cout << "Total Flod id " << j << std::endl;
  // (*monitor)->SerializeToXmlFile ("/home/elton/manet-routing_flow-monitor.xml", true, true);
  // std::cout << jout.dump (4) << std::endl;
  // --

  Simulator::Schedule (Seconds (1.0), &Scenario::MetricsHandler, this, jout, nodes_ip, flowmon,
                       monitor, Simulator::Now ().GetSeconds ());
}

void
Scenario::operator() ()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (CONFIGURATOR->IsDryRun ())
    return;

  std::stringstream progressLogFilePath;
  progressLogFilePath << CONFIGURATOR->GetResultsPath () << "progress.log";
  auto progressLogSink = Create<OutputStreamWrapper> (progressLogFilePath.str (), std::ios::out);

  ShowProgress progressLog{Seconds (PROGRESS_REFRESH_INTERVAL_SECONDS),
                           (*progressLogSink->GetStream ())};
  ShowProgress progressStdout{Seconds (PROGRESS_REFRESH_INTERVAL_SECONDS), std::cout};

  nlohmann::json nodes_ip;

  // TODO: refatorar
  // m_drones
  for (uint32_t i = 0; i < m_drones.GetN (); i++)
    {
      auto drone = m_drones.Get (i);
      auto ipv4 = drone->GetObject<Ipv4> ();
      for (uint32_t j = 1; j < ipv4->GetNInterfaces (); j++)
        {
          std::stringstream ip_address;
          ip_address << ipv4->GetAddress (j, 0).GetAddress ();
          // nodes_ip[ip_address.str ()] = "drone" + std::to_string (drone->GetId ()) + "_if" + std::to_string (j);
          nodes_ip[ip_address.str ()] = nlohmann::json::object (
              {{"type", "drone"},
               {"id", drone->GetId ()},
               {"iface", "if" + std::to_string (j)},
               {"label", "drone" + std::to_string (drone->GetId ()) + "_if" + std::to_string (j)},
               {"cIndex", i}});
        }
    }
  // m_zsps;
  for (uint32_t i = 0; i < m_zsps.GetN (); i++)
    {
      auto zsp = m_zsps.Get (i);
      auto ipv4 = zsp->GetObject<Ipv4> ();
      for (uint32_t j = 1; j < ipv4->GetNInterfaces (); j++)
        {
          std::stringstream ip_address;
          ip_address << ipv4->GetAddress (j, 0).GetAddress ();
          // nodes_ip[ip_address.str ()] = "zsp" + std::to_string (zsp->GetId ()) + "_if" + std::to_string (j);
          nodes_ip[ip_address.str ()] = nlohmann::json::object (
              {{"type", "zsp"},
               {"id", zsp->GetId ()},
               {"iface", "if" + std::to_string (j)},
               {"label", "zsp" + std::to_string (zsp->GetId ()) + "_if" + std::to_string (j)},
               {"cIndex", i}});
        }
    }
  // m_remoteNodes;
  for (uint32_t i = 0; i < m_remoteNodes.GetN (); i++)
    {
      auto remoteNode = m_remoteNodes.Get (i);
      auto ipv4 = remoteNode->GetObject<Ipv4> ();
      for (uint32_t j = 1; j < ipv4->GetNInterfaces (); j++)
        {
          std::stringstream ip_address;
          ip_address << ipv4->GetAddress (j, 0).GetAddress ();
          // nodes_ip[ip_address.str ()] = "remoteNode" + std::to_string (remoteNode->GetId ()) + "_if" + std::to_string (j);
          nodes_ip[ip_address.str ()] = nlohmann::json::object (
              {{"type", "remoteNode"},
               {"id", remoteNode->GetId ()},
               {"iface", "if" + std::to_string (j)},
               {"label",
                "remoteNode" + std::to_string (remoteNode->GetId ()) + "_if" + std::to_string (j)},
               {"cIndex", i}});
        }
    }
  // m_backbone;
  for (uint32_t i = 0; i < m_backbone.GetN (); i++)
    {
      auto backbone = m_backbone.Get (i);
      auto ipv4 = backbone->GetObject<Ipv4> ();
      for (uint32_t j = 1; j < ipv4->GetNInterfaces (); j++)
        {
          std::stringstream ip_address;
          ip_address << ipv4->GetAddress (j, 0).GetAddress ();
          // nodes_ip[ip_address.str ()] = "backbone" + std::to_string (backbone->GetId ()) + "_if" + std::to_string (j);
          nodes_ip[ip_address.str ()] =
              nlohmann::json::object ({{"type", "backbone"},
                                       {"id", backbone->GetId ()},
                                       {"iface", "if" + std::to_string (j)},
                                       {"label", "backbone" + std::to_string (backbone->GetId ()) +
                                                     "_if" + std::to_string (j)},
                                       {"cIndex", i}});
        }
    }

  // other nodes
  for (NodeList::Iterator i = NodeList::Begin (); i != NodeList::End (); ++i)
    {
      Ptr<Node> node = *i;
      if (node->GetObject<Ipv4L3Protocol> () || node->GetObject<Ipv6L3Protocol> ())
        {
          auto ipv4 = node->GetObject<Ipv4> ();
          for (uint32_t j = 1; j < ipv4->GetNInterfaces (); j++)
            {
              std::stringstream ip_address;
              ip_address << ipv4->GetAddress (j, 0).GetAddress ();
              if (nodes_ip.count (ip_address.str ()) == 0)
                // nodes_ip[ip_address.str ()] = "node" + std::to_string (node->GetId ()) + "_if" + std::to_string (j);
                nodes_ip[ip_address.str ()] =
                    nlohmann::json::object ({{"type", "node"},
                                             {"id", node->GetId ()},
                                             {"iface", "if" + std::to_string (j)},
                                             {"label", "node" + std::to_string (node->GetId ()) +
                                                           "_if" + std::to_string (j)}});
            }
        }
    }

  std::cout << nodes_ip.dump (4) << std::endl;

  // Flow Monitor 3
  ns3::FlowMonitorHelper flowmon;
  ns3::Ptr<ns3::FlowMonitor> monitor = flowmon.InstallAll ();

  // schedule metricshandler
  nlohmann::json jout;
  jout["flow-metrics"] = nlohmann::json::array ();
  jout["drones-metrics"] = nlohmann::json::array ();
  jout["zsps-metrics"] = nlohmann::json::array ();
  jout["nodes-metrics"] = nlohmann::json::array ();
  jout["remoteNodes-metrics"] = nlohmann::json::array ();

  Simulator::Schedule (Seconds (1.0), &Scenario::MetricsHandler, this, &jout, &nodes_ip, &flowmon,
                       &monitor, Simulator::Now ().GetSeconds ());

  Simulator::Run ();
  // Report Module needs the simulator context alive to introspect it

  // save json time series metrics to file
  std::stringstream timeSeries;
  timeSeries << CONFIGURATOR->GetResultsPath () << "time-series-metrics.json";
  std::ofstream o (timeSeries.str ());
  o << std::setw (4) << jout << std::endl;

  Report::Get ()->Save ();
  Simulator::Destroy ();
}

void
Scenario::ApplyStaticConfig ()
{
  NS_LOG_FUNCTION_NOARGS ();

  const auto staticConfig = CONFIGURATOR->GetStaticConfig ();
  for (auto &param : staticConfig)
    Config::SetDefault (param.first, StringValue (param.second));
}

void
Scenario::ConfigureWorld ()
{
  NS_LOG_FUNCTION_NOARGS ();

  CONFIGURATOR->GetBuildings (); // buildings created here are automatically added to BuildingsList
}

void
Scenario::ConfigurePhy ()
{
  NS_LOG_FUNCTION_NOARGS ();

  const auto phyLayerConfs = CONFIGURATOR->GetPhyLayers ();

  size_t phyId = 0;
  for (auto &phyLayerConf : phyLayerConfs)
    {
      if (phyLayerConf->GetType () == "wifi")
        {
          YansWifiChannelHelper wifiChannel;
          const auto wifiConf =
              StaticCast<WifiPhyLayerConfiguration, PhyLayerConfiguration> (phyLayerConf);
          const auto wifiSim = CreateObject<WifiPhySimulationHelper> ();
          YansWifiPhyHelper *wifiPhy = wifiSim->GetWifiPhyHelper ();

          wifiSim->GetWifiHelper ()->SetStandard (wifiConf->GetStandard ());

          for (auto &attr : wifiConf->GetAttributes ())
            wifiPhy->Set (attr.name, *attr.value);

          // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
          wifiPhy->SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

          WifiPhyFactoryHelper::SetPropagationDelay (wifiChannel,
                                                     wifiConf->GetChannelPropagationDelayModel ());
          WifiPhyFactoryHelper::AddPropagationLoss (wifiChannel,
                                                    wifiConf->GetChannelPropagationLossModel ());

          wifiPhy->SetChannel (wifiChannel.Create ());

          m_protocolStacks[PHY_LAYER].push_back (wifiSim);
        }
      else if (phyLayerConf->GetType () == "lte")
        {
          auto lteSim = CreateObject<LtePhySimulationHelper> (phyId);
          auto lteConf = StaticCast<LtePhyLayerConfiguration, PhyLayerConfiguration> (phyLayerConf);
          auto lteHelper = lteSim->GetLteHelper ();

          auto pathlossConf = lteConf->GetChannelPropagationLossModel ();
          lteHelper->SetAttribute ("PathlossModel", StringValue (pathlossConf.GetName ()));
          for (auto &attr : pathlossConf.GetAttributes ())
            lteHelper->SetPathlossModelAttribute (attr.name, *attr.value);

          auto spectrumConf = lteConf->GetChannelSpectrumModel ();
          lteHelper->SetSpectrumChannelType (spectrumConf.GetName ());
          for (auto &attr : spectrumConf.GetAttributes ())
            lteHelper->SetSpectrumChannelAttribute (attr.name, *attr.value);

          for (auto &attr : lteConf->GetAttributes ())
            lteHelper->SetAttribute (attr.name, *attr.value);

          lteHelper->Initialize ();

          m_backbone.Add (lteSim->GetEpcHelper ()->GetPgwNode ());

          m_protocolStacks[PHY_LAYER].push_back (lteSim);
        }
      else
        {
          NS_FATAL_ERROR ("Unsupported PHY Layer Type: " << phyLayerConf->GetType ());
        }

      phyId++;
    }
}

void
Scenario::ConfigureMac ()
{
  NS_LOG_FUNCTION_NOARGS ();

  const auto macLayerConfs = CONFIGURATOR->GetMacLayers ();

  size_t i = 0;
  for (auto &macLayerConf : macLayerConfs)
    {
      if (macLayerConf->GetType () == "wifi")
        {
          const auto wifiPhy =
              StaticCast<WifiPhySimulationHelper, Object> (m_protocolStacks[PHY_LAYER][i]);
          const auto wifiMac = CreateObject<WifiMacSimulationHelper> ();
          const auto wifiConf =
              StaticCast<WifiMacLayerConfiguration, MacLayerConfiguration> (macLayerConf);
          Ssid ssid = Ssid (wifiConf->GetSsid ());

          WifiMacFactoryHelper::SetRemoteStationManager (
              *(wifiPhy->GetWifiHelper ()), wifiConf->GetRemoteStationManagerConfiguration ());

          m_protocolStacks[MAC_LAYER].push_back (wifiMac);
        }
      else if (macLayerConf->GetType () == "lte")
        {
          // NO OPERATION NEEDED HERE
          m_protocolStacks[MAC_LAYER].push_back (nullptr);
        }
      else
        {
          NS_FATAL_ERROR ("Unsupported MAC Layer Type: " << macLayerConf->GetType ());
        }

      i++;
    }
}

void
Scenario::ConfigureNetwork ()
{
  NS_LOG_FUNCTION_NOARGS ();

  const auto layerConfs = CONFIGURATOR->GetNetworkLayers ();
  for (auto &layerConf : layerConfs)
    {
      if (layerConf->GetType () == "ipv4")
        {
          const auto ipv4Conf =
              StaticCast<Ipv4NetworkLayerConfiguration, NetworkLayerConfiguration> (layerConf);
          const auto ipv4Sim = CreateObject<Ipv4SimulationHelper> (ipv4Conf->GetMask (),
                                                                   ipv4Conf->GetGatewayAddress ());

          ipv4Sim->GetIpv4Helper ().SetBase (Ipv4Address (ipv4Conf->GetAddress ().c_str ()),
                                             Ipv4Mask (ipv4Conf->GetMask ().c_str ()));

          EnableIpv4RoutingTableReporting ();

          m_protocolStacks[NET_LAYER].push_back (ipv4Sim);
        }
      else
        {
          NS_FATAL_ERROR ("Unsupported Network Layer Type: " << layerConf->GetType ());
        }
    }
}

void
Scenario::EnableIpv4RoutingTableReporting ()
{
  // this method should be called once
  static bool hasBeenCalled = false;
  if (hasBeenCalled)
    return;
  else
    hasBeenCalled = true;

  NS_LOG_FUNCTION_NOARGS ();

  std::stringstream routingTableFilePath;
  routingTableFilePath << CONFIGURATOR->GetResultsPath () << "ipv4-routing-tables.txt";
  auto routingTableSink = Create<OutputStreamWrapper> (routingTableFilePath.str (), std::ios::out);
  Ipv4RoutingHelper::PrintRoutingTableAllAt (Seconds (1.0), routingTableSink);
}

void
Scenario::ConfigureEntities (const std::string &entityKey, NodeContainer &nodes)
{
  NS_LOG_FUNCTION (entityKey);

  const auto entityConfs = CONFIGURATOR->GetEntitiesConfiguration (entityKey);
  size_t entityId = 0;

  for (auto &entityConf : entityConfs)
    {
      size_t deviceId = 0;

      auto entityNode = nodes.Get (entityId);
      ConfigureEntityMobility (entityKey, entityConf, entityId);

      for (auto &entityNetDev : entityConf->GetNetDevices ())
        {
          const auto netId = entityNetDev->GetNetworkLayerId ();

          if (entityNetDev->GetType () == "wifi")
            {
              auto devContainer = ConfigureEntityWifiStack (entityKey, entityNetDev, entityNode,
                                                            entityId, deviceId, netId);
              InstallEntityIpv4 (entityNode, devContainer, netId);
              ConfigureEntityIpv4 (entityNode, devContainer, deviceId, netId);
            }
          else if (entityNetDev->GetType () == "lte")
            {
              const auto entityLteDevConf =
                  StaticCast<LteNetdeviceConfiguration, NetdeviceConfiguration> (entityNetDev);
              const auto role = entityLteDevConf->GetRole ();

              switch (role)
                {
                case eNB:
                  ConfigureLteEnb (entityNode, netId);
                  break;
                case UE:
                  ConfigureLteUe (entityNode, entityLteDevConf->GetBearers (), netId);
                  break;
                default:
                  NS_FATAL_ERROR ("Unrecognized LTE role for entity ID " << entityId);
                }
            }
          else
            {
              NS_FATAL_ERROR (
                  "Unsupported Drone Network Device Type: " << entityNetDev->GetType ());
            }

          deviceId++;
        }

      ConfigureEntityApplications (entityKey, entityConf, entityId);

      if (entityKey == "drones")
        {
          DroneEnergyModelHelper energyModel;
          Ptr<EnergySource> energySource;

          ConfigureEntityMechanics (entityKey, entityConf, entityId);
          energySource = ConfigureEntityBattery (entityKey, entityConf, entityId);
          /// Installing Energy Model on Drone
          energyModel.Install (StaticCast<Drone, Node> (entityNode), energySource);
          ConfigureEntityPeripherals (entityKey, entityConf, entityId);
        }

      entityId++;
    }

  BuildingsHelper::Install (nodes);
}

void
Scenario::ConfigureEntityMobility (const std::string &entityKey,
                                   Ptr<EntityConfiguration> entityConf, const uint32_t entityId)
{
  NS_LOG_FUNCTION (entityKey << entityConf << entityId);

  MobilityHelper mobility;
  const auto mobilityConf = entityConf->GetMobilityModel ();
  const auto mobilityType = mobilityConf.GetName (); // Configure Entity Mobility
  MobilityFactoryHelper::SetMobilityModel (mobility, mobilityConf);

  // const auto initialPosition = mobilityConf.GetInitialPosition ();
  // if (initialPosition)

  if (entityKey == "drones")
    {
      mobility.Install (m_drones.Get (entityId));
      std::ostringstream oss;
      oss << "/NodeList/" << m_drones.Get (entityId)->GetId ()
          << "/$ns3::MobilityModel/CourseChange";
      Config::Connect (oss.str (), MakeCallback (&Scenario::CourseChange, this));
    }
  else if (entityKey == "ZSPs")
    {
      mobility.Install (m_zsps.Get (entityId));
    }
}

NetDeviceContainer
Scenario::ConfigureEntityWifiStack (const std::string entityKey,
                                    Ptr<NetdeviceConfiguration> entityNetDev, Ptr<Node> entityNode,
                                    const uint32_t entityId, const uint32_t deviceId,
                                    const uint32_t netId)
{
  NS_LOG_FUNCTION (entityNetDev << entityNode << entityId << deviceId << netId);

  auto wifiPhy = StaticCast<WifiPhySimulationHelper, Object> (m_protocolStacks[PHY_LAYER][netId]);
  auto wifiMac = StaticCast<WifiMacSimulationHelper, Object> (m_protocolStacks[MAC_LAYER][netId]);
  auto wifiNetDev = StaticCast<WifiNetdeviceConfiguration, NetdeviceConfiguration> (entityNetDev);
  auto wifiMacAttrs = wifiNetDev->GetMacLayer ().GetAttributes ();

  if (wifiMacAttrs.size () == 0)
    wifiMac->GetMacHelper ().SetType (wifiNetDev->GetMacLayer ().GetName ());
  else if (wifiMacAttrs.size () == 1)
    wifiMac->GetMacHelper ().SetType (wifiNetDev->GetMacLayer ().GetName (), wifiMacAttrs[0].name,
                                      *wifiMacAttrs[0].value);

  NetDeviceContainer devContainer = wifiPhy->GetWifiHelper ()->Install (
      *wifiPhy->GetWifiPhyHelper (), wifiMac->GetMacHelper (), entityNode);

  // Configure WiFi PHY Logging
  std::stringstream phyTraceLog;
  std::stringstream pcapLog;
  AsciiTraceHelper ascii;

  // Configure WiFi TXT PHY Logging
  phyTraceLog << CONFIGURATOR->GetResultsPath () << "wifi-phy-" << netId << "-" << entityKey
              << "-host-" << entityId << "-" << deviceId << ".log";
  wifiPhy->GetWifiPhyHelper ()->EnableAscii (ascii.CreateFileStream (phyTraceLog.str ()),
                                             entityNode->GetId (),
                                             devContainer.Get (0)->GetIfIndex ());

  // Configure WiFi PCAP Logging
  pcapLog << CONFIGURATOR->GetResultsPath () << "wifi-phy-" << netId << "-" << entityKey << "-host";
  wifiPhy->GetWifiPhyHelper ()->EnablePcap (pcapLog.str (), entityNode->GetId (),
                                            devContainer.Get (0)->GetIfIndex ());

  return devContainer;
}

void
Scenario::ConfigureLteEnb (Ptr<Node> entityNode, const uint32_t netId)
{
  // !NOTICE: no checks are made for backbone/netid combination that do not represent an LTE backbone!
  static std::vector<NodeContainer> backbonePerStack (m_protocolStacks[PHY_LAYER].size ());
  auto ltePhy = StaticCast<LtePhySimulationHelper, Object> (m_protocolStacks[PHY_LAYER][netId]);

  auto dev = StaticCast<LteEnbNetDevice, NetDevice> (
      LteSetupHelper::InstallSingleEnbDevice (ltePhy->GetLteHelper (), entityNode));
  dev->GetPhy ()->SetAttribute (
      "TxPower",
      DoubleValue (
          49.0)); // TODO: How can we place it on the JSON? - Ref: https://www.itu.int/dms_pub/itu-r/opb/rep/R-REP-M.2135-1-2009-PDF-E.pdf

  for (NodeContainer::Iterator eNB = backbonePerStack[netId].Begin ();
       eNB != backbonePerStack[netId].End (); eNB++)
    ltePhy->GetLteHelper ()->AddX2Interface (entityNode, *eNB);
  backbonePerStack[netId].Add (entityNode);
}

void
Scenario::ConfigureLteUe (Ptr<Node> entityNode, const std::vector<LteBearerConfiguration> bearers,
                          const uint32_t netId)
{
  // NOTICE: no checks are made for ue/netid combination that do not represent an LTE backbone!
  static std::vector<NodeContainer> uePerStack (m_protocolStacks[PHY_LAYER].size ());
  auto ltePhy = StaticCast<LtePhySimulationHelper, Object> (m_protocolStacks[PHY_LAYER][netId]);
  Ipv4StaticRoutingHelper routingHelper;

  auto dev = StaticCast<LteUeNetDevice, NetDevice> (
      LteSetupHelper::InstallSingleUeDevice (ltePhy->GetLteHelper (), entityNode));
  dev->GetPhy ()->SetAttribute (
      "TxPower",
      DoubleValue (
          24.0)); // TODO: How can we place it on the JSON? - Ref: https://www.itu.int/dms_pub/itu-r/opb/rep/R-REP-M.2135-1-2009-PDF-E.pdf

  // Install network layer in order to proceed with IPv4 LTE configuration
  InstallEntityIpv4 (entityNode, dev, netId);
  // Register UEs into network 7.0.0.0/8
  // unfortunately this is hardwired into EpcHelper implementation

  auto assignedIpAddrs = ltePhy->GetEpcHelper ()->AssignUeIpv4Address (NetDeviceContainer (dev));
  for (auto assignedIpIter = assignedIpAddrs.Begin (); assignedIpIter != assignedIpAddrs.End ();
       assignedIpIter++)
    {
      NS_LOG_LOGIC ("Assigned IPv4 Address to UE with Node ID " << entityNode->GetId () << ":"
                                                                << " Iface "
                                                                << assignedIpIter->second);

      for (uint32_t i = 0; i < assignedIpIter->first->GetNAddresses (assignedIpIter->second); i++)
        {
          NS_LOG_LOGIC (" Addr " << assignedIpIter->first->GetAddress (assignedIpIter->second, i));
        }
    }

  // create a static route for each UE to the SGW/PGW in order to communicate
  // with the internet
  auto nodeIpv4 = entityNode->GetObject<Ipv4> ();
  Ptr<Ipv4StaticRouting> ueStaticRoute = routingHelper.GetStaticRouting (nodeIpv4);
  ueStaticRoute->SetDefaultRoute (ltePhy->GetEpcHelper ()->GetUeDefaultGatewayAddress (),
                                  nodeIpv4->GetInterfaceForDevice (dev));
  // auto attach each drone UE to the eNB with the strongest signal
  ltePhy->GetLteHelper ()->Attach (dev);
  // init bearers on UE
  for (auto &bearerConf : bearers)
    {
      EpsBearer bearer (bearerConf.GetType (), bearerConf.GetQos ());
      ltePhy->GetLteHelper ()->ActivateDedicatedEpsBearer (dev, bearer, EpcTft::Default ());
    }
}

void
Scenario::InstallEntityIpv4 (Ptr<Node> entityNode, NetDeviceContainer netDevices,
                             const uint32_t netId)
{
  NS_LOG_FUNCTION (entityNode << netId);

  for (NetDeviceContainer::Iterator i = netDevices.Begin (); i != netDevices.End (); i++)
    InstallEntityIpv4 (entityNode, *i, netId);
}

void
Scenario::InstallEntityIpv4 (Ptr<Node> entityNode, Ptr<NetDevice> netDevice, const uint32_t netId)
{
  NS_LOG_FUNCTION (entityNode << netId);

  auto ipv4Obj = entityNode->GetObject<Ipv4> ();

  if (ipv4Obj == nullptr)
    {
      auto netLayer = StaticCast<Ipv4SimulationHelper, Object> (m_protocolStacks[NET_LAYER][netId]);
      netLayer->GetInternetHelper ().Install (entityNode);
    }
  else
    {
      ipv4Obj->AddInterface (netDevice);
    }
}

void
Scenario::ConfigureEntityIpv4 (Ptr<Node> entityNode, NetDeviceContainer devContainer,
                               const uint32_t deviceId, const uint32_t netId)
{
  NS_LOG_FUNCTION (entityNode << deviceId << netId);
  NS_ASSERT_MSG (1 == devContainer.GetN (),
                 "ConfigureEntityIpv4 assumes to receive a NetDeviceContainer with only one "
                 "NetDevice, but there are "
                     << devContainer.GetN ());

  auto netLayer = StaticCast<Ipv4SimulationHelper, Object> (m_protocolStacks[NET_LAYER][netId]);
  auto assignedIPs = netLayer->GetIpv4Helper ().Assign (devContainer);

  Ipv4StaticRoutingHelper routingHelper;
  auto deviceIP = assignedIPs.Get (0);
  Ptr<Ipv4StaticRouting> ueStaticRoute = routingHelper.GetStaticRouting (deviceIP.first);

  const Ipv4Address nodeAddr = assignedIPs.GetAddress (0, 0);
  if (nodeAddr != netLayer->GetGatewayAddress ())
    ueStaticRoute->SetDefaultRoute (netLayer->GetGatewayAddress (), deviceIP.second);
}

void
Scenario::ConfigureEntityApplications (const std::string &entityKey,
                                       const Ptr<EntityConfiguration> &conf,
                                       const uint32_t &entityId)
{
  NS_LOG_FUNCTION (entityKey << conf << entityId);

  for (auto appConf : conf->GetApplications ())
    {
      ObjectFactory f{appConf.GetName ()};

      for (auto attr : appConf.GetAttributes ())
        f.Set (attr.name, *attr.value);

      auto app = StaticCast<Application, Object> (f.Create ());

      if (entityKey == "drones")
        m_drones.Get (entityId)->AddApplication (app);
      else if (entityKey == "ZSPs")
        m_zsps.Get (entityId)->AddApplication (app);
      else
        NS_FATAL_ERROR ("Unsupported Entity Type " << entityKey);
    }
}

void
Scenario::ConfigureEntityMechanics (const std::string &entityKey,
                                    Ptr<EntityConfiguration> entityConf, const uint32_t entityId)
{
  NS_LOG_FUNCTION_NOARGS ();
  const auto mechanics = entityConf->GetMechanics ();
  for (auto attr : mechanics.GetAttributes ())
    m_drones.Get (entityId)->SetAttribute (attr.name, *attr.value);
}

Ptr<EnergySource>
Scenario::ConfigureEntityBattery (const std::string &entityKey, Ptr<EntityConfiguration> entityConf,
                                  const uint32_t entityId)
{
  NS_LOG_FUNCTION_NOARGS ();
  const auto battery = entityConf->GetBattery ();
  ObjectFactory batteryFactory;
  batteryFactory.SetTypeId (entityConf->GetBattery ().GetName ());

  for (auto attr : battery.GetAttributes ())
    batteryFactory.Set (attr.name, *attr.value);
  auto mountedBattery = batteryFactory.Create<EnergySource> ();

  mountedBattery->SetNode (m_drones.Get (entityId));
  m_drones.Get (entityId)->AggregateObject (mountedBattery);
  return mountedBattery;
}

void
Scenario::ConfigureEntityPeripherals (const std::string &entityKey,
                                      const Ptr<EntityConfiguration> &conf,
                                      const uint32_t &entityId)
{
  NS_LOG_FUNCTION (entityKey << entityId << conf);
  auto dronePeripheralsContainer = m_drones.Get (entityId)->getPeripherals ();
  if (conf->GetPeripherals ().size () == 0)
    return;
  for (auto perConf : conf->GetPeripherals ())
    {
      dronePeripheralsContainer->Add (perConf.GetName ());
      for (auto attr : perConf.GetAttributes ())
        dronePeripheralsContainer->Set (attr.name, *attr.value);
      auto peripheral = dronePeripheralsContainer->Create ();
      for (uint32_t i = 0; i < (uint32_t) peripheral->GetNRoI (); i++)
        {
          auto reg = irc->GetRoI (i);
          if (!irc->GetRoI (i))
            NS_FATAL_ERROR ("Region of Interest #" << i << " does not exist.");
        }
    }
  dronePeripheralsContainer->InstallAll (m_drones.Get (entityId));
}

void
Scenario::ConfigureInternetRemotes ()
{
  NS_LOG_FUNCTION_NOARGS ();

  const auto remoteConfs = CONFIGURATOR->GetRemotesConfiguration ();
  size_t remoteId = 0;

  for (auto &conf : remoteConfs)
    {
      const auto appConfs = conf->GetApplications ();

      for (auto &appConf : appConfs)
        {
          TypeId appTid;
          NS_ASSERT_MSG (TypeId::LookupByNameFailSafe (appConf.GetName (), &appTid),
                         "Failed to initialize application " << appConf.GetName ()
                                                             << ". It does not exist!");

          ObjectFactory factory (appTid.GetName ());

          for (auto &appAttr : appConf.GetAttributes ())
            factory.Set (appAttr.name, *appAttr.value);

          auto app = StaticCast<Application, Object> (factory.Create ());
          m_remoteNodes.Get (remoteId)->AddApplication (app);
        }

      remoteId++;
    }
}

void
Scenario::ConfigureInternetBackbone ()
{
  NS_LOG_FUNCTION_NOARGS ();

  InternetStackHelper internetHelper;
  Ipv4StaticRoutingHelper routingHelper;

  internetHelper.Install (m_remoteNodes);

  // setup a CSMA LAN between all the remotes and network gateways in the backbone
  CsmaHelper csma;
  NetDeviceContainer backboneDevs = csma.Install (m_backbone);

  // set a new address base for the backbone
  Ipv4AddressHelper ipv4H;
  ipv4H.SetBase (Ipv4Address ("200.0.0.0"), Ipv4Mask ("255.0.0.0"));
  ipv4H.Assign (backboneDevs);

  //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // create static routes between each remote node to each network gateway
  internetHelper.SetRoutingHelper (routingHelper);

  for (uint32_t i = 0; i < m_remoteNodes.GetN (); i++)
    {
      Ptr<Node> remoteNode = m_backbone.Get (i);
      for (uint32_t j = m_remoteNodes.GetN (); j < m_backbone.GetN (); j++)
        {
          Ptr<Node> netGwNode = m_backbone.Get (j);
          uint32_t netGwBackboneDevIndex = netGwNode->GetNDevices () - 1;

          // since network gateway have at least 2 ipv4 addresses (one in the network and the other for the backbone)
          // we should create a route to the internal ip using the backbone ip as next hop
          Ipv4Address netGwBackboneIpv4 =
              netGwNode->GetObject<Ipv4> ()->GetAddress (netGwBackboneDevIndex, 0).GetLocal ();
          Ipv4Address netGwInternalIpv4 =
              netGwNode->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();

          NS_LOG_LOGIC ("Setting-up static route: REMOTE "
                        << i << " "
                        << "(" << remoteNode->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ()
                        << ") "
                        << "-> NETWORK " << j - m_remoteNodes.GetN () << " "
                        << "(" << netGwBackboneIpv4 << " -> " << netGwInternalIpv4 << ")");

          Ptr<Ipv4StaticRouting> staticRouteRem =
              routingHelper.GetStaticRouting (remoteNode->GetObject<Ipv4> ());
          staticRouteRem->AddNetworkRouteTo (netGwInternalIpv4, Ipv4Mask ("255.0.0.0"),
                                             netGwBackboneIpv4, 1);
        }
    }

  std::stringstream logFilePathBuilder;
  logFilePathBuilder << CONFIGURATOR->GetResultsPath () << "internet";
  const auto logFilePath = logFilePathBuilder.str ();

  csma.EnablePcapAll (logFilePath, true);
  csma.EnableAsciiAll (logFilePath);
}

void
Scenario::EnablePhyLteTraces ()
{
  NS_LOG_FUNCTION_NOARGS ();

  for (size_t phyId = 0; phyId < m_protocolStacks[PHY_LAYER].size (); phyId++)
    {
      auto obj = m_protocolStacks[PHY_LAYER][phyId];

      if (typeid (*obj) == typeid (LtePhySimulationHelper))
        {
          /* LteHelperQuirk:
           *  This class is an hack to allow access to private members of LteHelper class,
           *  in particular to the StatsCalculators in order to set their output path.
           *  A structurally identical class is defined with all attributes set to public,
           *  then with a reinterpret_cast we interpret the LteHelper as this new class
           *  so the compiler won't complain about accessing its private members.
           */
          class LteHelperQuirk : public Object
          {
          public:
            Ptr<SpectrumChannel> dlC, ulC;
            Ptr<Object> dlPlM, ulPlM;
            ObjectFactory a, b, c, d, e, f, g, h, i, j, k;
            std::string fMT;
            ObjectFactory fMF;
            Ptr<SpectrumPropagationLossModel> fM;
            bool fSA;
            Ptr<PhyStatsCalculator> phyStat;
            Ptr<PhyTxStatsCalculator> phyTxStat;
            Ptr<PhyRxStatsCalculator> phyRxStat;
            Ptr<MacStatsCalculator> macStat;
            Ptr<RadioBearerStatsCalculator> rlcStat;
            Ptr<RadioBearerStatsCalculator> pdcpStat;
            RadioBearerStatsConnector radioBearerStatsConnector;
            Ptr<EpcHelper> m_epcHelper;
            uint64_t m_imsiCounter;
            uint16_t m_cellIdCounter;
            bool l, m, n, o;
            std::map<uint8_t, ComponentCarrier> m_componentCarrierPhyParams;
            uint16_t m_noOfCcs;
          };

          auto phy = StaticCast<LtePhySimulationHelper, Object> (obj);
          auto lteHelper = phy->GetLteHelper ();
          std::stringstream basePath;

          basePath << CONFIGURATOR->GetResultsPath () << "lte-" << phyId << "-";

          lteHelper->EnableTraces ();

          auto rlcStat = lteHelper->GetRlcStats ();
          rlcStat->SetDlOutputFilename (basePath.str () + "RlcDlStats.txt");
          rlcStat->SetUlOutputFilename (basePath.str () + "RlcUlStats.txt");

          auto pdcpStat = lteHelper->GetPdcpStats ();
          pdcpStat->SetDlPdcpOutputFilename (basePath.str () + "PdcpDlStats.txt");
          pdcpStat->SetUlPdcpOutputFilename (basePath.str () + "PdcpUlStats.txt");

          auto lteHelperQ = reinterpret_cast<LteHelperQuirk *> (&(*lteHelper));

          lteHelperQ->phyStat->SetUeSinrFilename (basePath.str () + "PhySinrUlStats.txt");
          lteHelperQ->phyStat->SetInterferenceFilename (basePath.str () +
                                                        "PhyInterferenceUlStats.txt");
          lteHelperQ->phyStat->SetCurrentCellRsrpSinrFilename (basePath.str () +
                                                               "PhyRsrpSinrDlStats.txt");

          lteHelperQ->phyRxStat->SetDlRxOutputFilename (basePath.str () + "PhyRxDlStats.txt");
          lteHelperQ->phyRxStat->SetUlRxOutputFilename (basePath.str () + "PhyRxUlStats.txt");

          lteHelperQ->phyTxStat->SetDlTxOutputFilename (basePath.str () + "PhyTxDlStats.txt");
          lteHelperQ->phyTxStat->SetUlTxOutputFilename (basePath.str () + "PhyTxUlStats.txt");

          lteHelperQ->macStat->SetDlOutputFilename (basePath.str () + "MacDlStats.txt");
          lteHelperQ->macStat->SetUlOutputFilename (basePath.str () + "MacUlStats.txt");
        }
    }
}

void
Scenario::ConfigureRegionsOfInterest ()
{
  const auto regions = CONFIGURATOR->GetRegionsOfInterest ();
  Ptr<InterestRegion> reg;
  for (auto region : regions)
    {
      reg = irc->Create (region);
    }
}

void
Scenario::CourseChange (std::string context, Ptr<const MobilityModel> model)
{
  Vector position = model->GetPosition ();
  std::string start = "/NodeList/";
  std::string end = "/$ns3::MobilityModel/CourseChange";
  std::string id = context.substr (context.find (start) + start.length (),
                                   context.length () - end.length () - start.length ());
  auto dronePeripheralsContainer = m_drones.Get (std::stoi (id))->getPeripherals ();
  Ptr<DronePeripheral> peripheral;
  std::vector<int> regionindex;
  for (DronePeripheralContainer::Iterator i = dronePeripheralsContainer->Begin ();
       i != dronePeripheralsContainer->End (); i++)
    {
      peripheral = *i;
      regionindex = peripheral->GetRegionsOfInterest ();
      int status = irc->IsInRegions (regionindex, position);
      if (regionindex.empty ())
        continue;
      if (status >= 0 || status == -2)
        {
          if (peripheral->GetState () != DronePeripheral::PeripheralState::ON)
            peripheral->SetState (DronePeripheral::PeripheralState::ON);
        }
      else
        {
          if (peripheral->GetState () == DronePeripheral::PeripheralState::ON)
            peripheral->SetState (DronePeripheral::PeripheralState::IDLE);
        }
    }
}

void
Scenario::ConfigureSimulator ()
{
  NS_LOG_FUNCTION_NOARGS ();

  // Enable Report
  Report::Get ()->Initialize (CONFIGURATOR->GetName (), CONFIGURATOR->GetCurrentDateTime (),
                              CONFIGURATOR->GetResultsPath ());

  Simulator::Stop (Seconds (CONFIGURATOR->GetDuration ()));
}

} // namespace ns3

// Scenario Context
// context
class ScenarioContext
{ // The class
private:
  void initScenario (std::string req_body);

public: // Access specifier
  std::string status; // Attribute (string variable)
  // ns3::Scenario scenario;
  pthread_t t_handler;

  std::string
  startSimulation (std::string req_body)
  {
    std::cout << "New Simulation | status=" << status << std::endl;
    // std::cout << "req_body: " << req_body << std::endl;
    // std::cout << "Starting Simulation | status=" << status << std::endl;
    std::thread s_thread (&ScenarioContext::initScenario, this, req_body);
    t_handler = s_thread.native_handle ();
    s_thread.detach ();
    return "Simulation Started.";
  }

  std::string
  stopSimulation ()
  {
    if (status != "Running")
      {
        return "Simulation not running";
      }
    std::cout << "Stopping Simulation | status=" << status << std::endl;
    pthread_cancel (t_handler);
    status = "Stopped";
    return "Simulation Stopped";
  }
};

void
ScenarioContext::initScenario (std::string req_body)
{
  std::cout << "req_body: " << req_body << std::endl;
  ns3::Scenario s (req_body);
  // s ();  // run the scenario as soon as it is ready
  status = "Running";
  s ();
  status = "Ended";
}

int
main (int argc, char **argv)
{
  // crow
  crow::SimpleApp app;
  ScenarioContext sc;

  bool created = false;

  CROW_ROUTE (app, "/simulation")
  ([] () {
    // Simulator::Run ();
    // Simulator::Destroy ();
    return "Simulation Done";
  });

  CROW_ROUTE (app, "/simulation").methods (crow::HTTPMethod::POST) ([&] (const crow::request &req) {
    auto body = crow::json::load (req.body);
    if (created)
      return crow::response (400, "Simulation is already running");
    if (!body)
      return crow::response (400, "Invalid body");
    created = true;

    sc.startSimulation (req.body);

    return crow::response (200, "Simulation started");
  });

  CROW_ROUTE (app, "/simulation")
      .methods (crow::HTTPMethod::DELETE) ([&] (const crow::request &req) {
        auto body = crow::json::load (req.body);

        sc.stopSimulation ();

        return crow::response (200, "Simulation stopped");
      });

  app.port (18080).multithreaded ().run ();

  return EXIT_SUCCESS;

  return 0;
}

// context:
// - simulation
// - status (starting, configuring, running, ended)
// - thread
