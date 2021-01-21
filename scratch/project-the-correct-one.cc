/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

//
// Network topology
//
//  n0
//     \ 5 Mb/s, 2ms
//      \          1.5Mb/s, 10ms
//       n2 -------------------------n3
//      /
//     / 5 Mb/s, 2ms
//   n1
//
// - all links are point-to-point links with indicated one-way BW/delay
// - CBR/UDP flows from n0 to n3, and from n3 to n1
// - FTP/TCP flow from n0 to n3, starting at time 1.2 to time 1.35 sec.
// - UDP packet size of 210 bytes, with per-packet interval 0.00375 sec.
//   (i.e., DataRate of 448,000 bps)
// - DropTail queues
// - Tracing of queues and packet receptions to file "simple-global-routing.tr"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimpleGlobalRoutingExample");

void TcPacketsInQueue(QueueDiscContainer qdiscs,
                      Ptr<OutputStreamWrapper> stream) {

  uint32_t nQueueDiscs = qdiscs.GetN();
  for (uint32_t i = 0; i < nQueueDiscs; ++i) {
    Ptr<QueueDisc> p = qdiscs.Get(i);
    uint32_t size = p->GetNPackets();
    *stream->GetStream() << Simulator::Now().GetSeconds() << "\t" << size
                         << std::endl;
  }
  // Get current queue size value and save to file.
  //	Ptr<QueueDisc> p = qdiscs.Get (0);
  //	uint32_t size = p->GetNPackets();
  //	*stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << size
  //<< std::endl;
}

static void received_msg(Ptr<Socket> socket1, Ptr<Socket> socket2,
                         Ptr<const Packet> p, const Address &srcAddress,
                         const Address &dstAddress) {
  //std::cout << "::::: A packet received at the Server! Time:   "
  //          << Simulator::Now().GetSeconds() << std::endl;

  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();

  if (rand->GetValue(0.0, 1.0) <= 0.7) {
    //std::cout << "::::: Transmitting from Server to Router   " << std::endl;
    socket1->Send(Create<Packet>(p->GetSize()));
  } else {
    //std::cout << "::::: Transmitting from GW to Controller   " << std::endl;
    socket2->SendTo(Create<Packet>(p->GetSize()), 0, srcAddress);
  }
}

static void GenerateTraffic(Ptr<Socket> socket,
                            Ptr<ExponentialRandomVariable> randomSize,
                            Ptr<ExponentialRandomVariable> randomTime) {
  uint32_t pktSize =
      randomSize->GetInteger(); // Get random value for packet size
  //std::cout << "::::: A packet is generate at Node "
            //<< socket->GetNode()->GetId() << " with size " << pktSize
            //<< " bytes ! Time:   " << Simulator::Now().GetSeconds()
            //<< std::endl;

  // We make sure that the message is at least 12 bytes. The minimum length of
  // the UDP header. We would get error otherwise.
  if (pktSize < 12) {
    pktSize = 12;
  }

  socket->Send(Create<Packet>(pktSize));

  Time pktInterval = Seconds(
      randomTime
          ->GetValue()); // Get random value for next packet generation time
  Simulator::Schedule(pktInterval, &GenerateTraffic, socket, randomSize,
                      randomTime); // Schedule next packet generation
}

int main(int argc, char *argv[]) {
  // Users may find it convenient to turn on explicit debugging
  // for selected modules; the below lines suggest how to do this
#if 0 
   LogComponentEnable ("SimpleGlobalRoutingExample", LOG_LEVEL_INFO);
#endif

  // Set up some default values for the simulation.  Use the
  Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(100));
  Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("50kb/s"));

  // DefaultValue::Bind ("DropTailQueue::m_maxPackets", 30);

  // Allow the user to override any of the defaults and the above
  // DefaultValue::Bind ()s at run-time, via command-line arguments
  // CommandLine cmd (__FILE__);
  bool enableFlowMonitor = true;
  // cmd.AddValue ("EnableMonitor", "Enable Flow Monitor", enableFlowMonitor);
  // cmd.Parse (argc, argv);

  double simulationTime = 11; // seconds
  std::string queueSize = "10000";

  // Here, we will explicitly create four nodes.  In more sophisticated
  // topologies, we could configure a node factory.
  NS_LOG_INFO("Create nodes.");
  NodeContainer c;
  Address serverAddress;

  TrafficControlHelper tch;
  // tch.Uninstall(dGdS);
  // uint16_t handle = tch.SetRootQueueDisc ("ns3::FifoQueueDisc");
  //  tch.AddInternalQueues (handle, 3, "ns3::DropTailQueue", "MaxSize",
  //  StringValue (queueSize+"p"));
  tch.SetRootQueueDisc("ns3::FifoQueueDisc", "MaxSize",
                       StringValue(queueSize + "p"));

  c.Create(10);
  NodeContainer nAnE = NodeContainer(c.Get(0), c.Get(1));
  NodeContainer nEnG = NodeContainer(c.Get(1), c.Get(2));

  NodeContainer nGnS = NodeContainer(c.Get(2), c.Get(3));
  NodeContainer nGnR = NodeContainer(c.Get(2), c.Get(8));

  NodeContainer nBnF = NodeContainer(c.Get(4), c.Get(5));
  NodeContainer nCnF = NodeContainer(c.Get(6), c.Get(5));

  NodeContainer nFnG = NodeContainer(c.Get(5), c.Get(2));
  NodeContainer nDnG = NodeContainer(c.Get(7), c.Get(2));

  InternetStackHelper internet;
  internet.Install(c);
  // We create the channels first without any IP addressing information
  NS_LOG_INFO("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));
  p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("10p"));
  
  
   CsmaHelper csma;
   csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
   csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
   csma.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("10p"));
  

   NetDeviceContainer dAdE = csma.Install(nAnE);
   NetDeviceContainer dEdG = csma.Install(nEnG);
   NetDeviceContainer dBdF = csma.Install(nBnF);
   NetDeviceContainer dCdF = csma.Install(nCnF);
   NetDeviceContainer dDdG = csma.Install(nDnG);

   csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (8000000)));
   csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
 

  NetDeviceContainer dFdG = csma.Install(nFnG);
  NetDeviceContainer dGdR = csma.Install(nGnR);

   csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (10000000)));
   csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

  NetDeviceContainer dGdS = csma.Install(nGnS);
  
  QueueDiscContainer qdiscs = tch.Install(dGdS);

  // Later, we add IP addresses.
  NS_LOG_INFO("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iAiE = ipv4.Assign(dAdE);

  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer iEiG = ipv4.Assign(dEdG);

  ipv4.SetBase("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer iBiF = ipv4.Assign(dBdF);

  ipv4.SetBase("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer iCiF = ipv4.Assign(dCdF);

  ipv4.SetBase("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer iFiG = ipv4.Assign(dFdG);

  ipv4.SetBase("10.1.6.0", "255.255.255.0");
  Ipv4InterfaceContainer iDiG = ipv4.Assign(dDdG);

  ipv4.SetBase("10.1.7.0", "255.255.255.0");
  Ipv4InterfaceContainer iGiR = ipv4.Assign(dGdR);

  ipv4.SetBase("10.1.8.0", "255.255.255.0");
  Ipv4InterfaceContainer iGiS = ipv4.Assign(dGdS);

  // Create router nodes, initialize routing database and set up the routing
  // tables in the nodes.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> stream =
      asciiTraceHelper.CreateFileStream("queue.tr");

  for (float t = 1.0; t < simulationTime; t += 0.001) {
    Simulator::Schedule(Seconds(t), &TcPacketsInQueue, qdiscs, stream);
  }

  NS_LOG_INFO("Create Applications.");
  //
  // Create a UdpServer application on node S.
  //
  uint16_t port_number = 9;
  ApplicationContainer server_apps;
  UdpServerHelper serverS(port_number);
  server_apps.Add(serverS.Install(c.Get(3)));

  Ptr<UdpServer> S1 = serverS.GetServer();

  // We Initialize the sockets responsable for transmitting messages

  TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");

  // Transmission Server -> Router
  Ptr<Socket> source1 = Socket::CreateSocket(c.Get(3), tid);
  InetSocketAddress remote1 =
      InetSocketAddress(iGiR.GetAddress(1), port_number);
  source1->Connect(remote1);

  // Transmission Server -> Client
  Ptr<Socket> source2 = Socket::CreateSocket(c.Get(3), tid);

  S1->TraceConnectWithoutContext(
      "RxWithAddresses", MakeBoundCallback(&received_msg, source1, source2));

  server_apps.Start(Seconds(1.0));
  server_apps.Stop(Seconds(10.0));

  //
  // Create a UdpServer application on node A,B.
  //
  UdpServerHelper server(port_number);
  server_apps.Add(server.Install(c.Get(0)));
  server_apps.Add(server.Install(c.Get(4)));

  // ####Alternative with Socket (i.e., exponential payload and
  // inter-transmission time)####

  Ptr<Socket> sourceA = Socket::CreateSocket(c.Get(0), tid);
  InetSocketAddress remote = InetSocketAddress(iGiS.GetAddress(1), port_number);
  sourceA->Connect(remote);

  Ptr<Socket> sourceB = Socket::CreateSocket(c.Get(4), tid);
  sourceB->Connect(remote);

  Ptr<Socket> sourceC = Socket::CreateSocket(c.Get(6), tid);
  sourceC->Connect(remote);

  Ptr<Socket> sourceD = Socket::CreateSocket(c.Get(7), tid);
  sourceD->Connect(remote);

  // Mean inter-transmission time
  double mean = 0.002; // 2 ms
  Ptr<ExponentialRandomVariable> randomTime =
      CreateObject<ExponentialRandomVariable>();
  randomTime->SetAttribute("Mean", DoubleValue(mean));

  double meanC = 0.0005; // 0.5 ms
  Ptr<ExponentialRandomVariable> randomTimeC =
      CreateObject<ExponentialRandomVariable>();
  randomTimeC->SetAttribute("Mean", DoubleValue(meanC));

  double meanD = 0.001; // 0.5 ms
  Ptr<ExponentialRandomVariable> randomTimeD =
      CreateObject<ExponentialRandomVariable>();
  randomTimeD->SetAttribute("Mean", DoubleValue(meanD));

  // Mean packet time
  double meanSize = 100; // 100 Bytes
  Ptr<ExponentialRandomVariable> randomSize =
      CreateObject<ExponentialRandomVariable>();
  randomSize->SetAttribute("Mean", DoubleValue(meanSize));

  Simulator::ScheduleWithContext(sourceA->GetNode()->GetId(), Seconds(2.0),
                                 &GenerateTraffic, sourceA, randomSize,
                                 randomTime);
  Simulator::ScheduleWithContext(sourceB->GetNode()->GetId(), Seconds(2.0),
                                 &GenerateTraffic, sourceB, randomSize,
                                 randomTime);
  Simulator::ScheduleWithContext(sourceC->GetNode()->GetId(), Seconds(2.0),
                                 &GenerateTraffic, sourceC, randomSize,
                                 randomTimeC);
  Simulator::ScheduleWithContext(sourceD->GetNode()->GetId(), Seconds(2.0),
                                 &GenerateTraffic, sourceD, randomSize,
                                 randomTimeD);

  //
  // Create a UdpEchoClient application to send UDP datagrams from node zero to
  // node one.
  //
  /*
         serverAddress = Address(  iGiS.GetAddress (1));
    uint32_t packetSize = 1024;
    //uint32_t maxPacketCount = 1;
    Time interPacketInterval = Seconds (1.);
    UdpEchoClientHelper client (serverAddress, port);
    //client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
    client.SetAttribute ("Interval", TimeValue (interPacketInterval));
    client.SetAttribute ("PacketSize", UintegerValue (packetSize));
    apps = client.Install (c.Get (0));
    apps.Start (Seconds (2.0));
    apps.Stop (Seconds (10.0));

 */

  //AsciiTraceHelper ascii;
  //csma.EnableAsciiAll(ascii.CreateFileStream("simple-global-routing.tr"));
  //csma.EnablePcapAll("simple-global-routing");



  // Flow Monitor
  FlowMonitorHelper flowmonHelper;
  if (enableFlowMonitor) {
    flowmonHelper.InstallAll();
  }

  NS_LOG_INFO("Run Simulation.");
  Simulator::Stop(Seconds(simulationTime));
  Simulator::Run();
  NS_LOG_INFO("Done.");

  if (enableFlowMonitor) {
    flowmonHelper.SerializeToXmlFile("simple-global-routing.flowmon", false,
                                     false);
  }

  Simulator::Destroy();
  return 0;
}
