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
 
 #include <iostream>
 #include <fstream>
 #include <string>
 #include <cassert>
 
 #include "ns3/core-module.h"
 #include "ns3/network-module.h"
 #include "ns3/internet-module.h"
 #include "ns3/point-to-point-module.h"
 #include "ns3/applications-module.h"
 #include "ns3/flow-monitor-helper.h"
 #include "ns3/ipv4-global-routing-helper.h"
 
 using namespace ns3;
 
 NS_LOG_COMPONENT_DEFINE ("SimpleGlobalRoutingExample");
 
 int 
 main (int argc, char *argv[])
 {
   // Users may find it convenient to turn on explicit debugging
   // for selected modules; the below lines suggest how to do this
 #if 0 
   LogComponentEnable ("SimpleGlobalRoutingExample", LOG_LEVEL_INFO);
 #endif
 
   // Set up some default values for the simulation.  Use the 
   Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (100));
   Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("50kb/s"));
 
   //DefaultValue::Bind ("DropTailQueue::m_maxPackets", 30);
 
   // Allow the user to override any of the defaults and the above
   // DefaultValue::Bind ()s at run-time, via command-line arguments
   //CommandLine cmd (__FILE__);
   bool enableFlowMonitor = true;
   //cmd.AddValue ("EnableMonitor", "Enable Flow Monitor", enableFlowMonitor);
   //cmd.Parse (argc, argv);
 
   // Here, we will explicitly create four nodes.  In more sophisticated
   // topologies, we could configure a node factory.
   NS_LOG_INFO ("Create nodes.");
   NodeContainer c;
   Address serverAddress;
   
   c.Create (10);
   NodeContainer nAnE = NodeContainer (c.Get (0), c.Get (1));
   NodeContainer nEnG = NodeContainer (c.Get (1), c.Get (2));
   
   NodeContainer nGnS = NodeContainer (c.Get (2), c.Get (3));
   NodeContainer nGnR = NodeContainer (c.Get (2), c.Get (8));
   
   NodeContainer nBnF = NodeContainer (c.Get (4), c.Get (5));
   NodeContainer nCnF = NodeContainer (c.Get (6), c.Get (5));
   
   NodeContainer nFnG = NodeContainer (c.Get (5), c.Get (2));
   NodeContainer nDnG = NodeContainer (c.Get (7), c.Get (2));
   
   
 
   InternetStackHelper internet;
   internet.Install (c);
 
   // We create the channels first without any IP addressing information
   NS_LOG_INFO ("Create channels.");
   PointToPointHelper p2p;
   p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
   p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
   
   NetDeviceContainer dAdE = p2p.Install (nAnE);
   NetDeviceContainer dEdG = p2p.Install (nEnG);
   NetDeviceContainer dBdF = p2p.Install (nBnF);
   NetDeviceContainer dCdF = p2p.Install (nCnF);
   NetDeviceContainer dDdG = p2p.Install (nDnG);
   
   
   p2p.SetDeviceAttribute ("DataRate", StringValue ("8Mbps"));
   p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
   
   NetDeviceContainer dFdG = p2p.Install (nFnG);
   NetDeviceContainer dGdR = p2p.Install (nGnR);
   
   
   p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
   p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
   
   NetDeviceContainer dGdS = p2p.Install (nGnS);
 
 
   // Later, we add IP addresses.
   NS_LOG_INFO ("Assign IP Addresses.");
   Ipv4AddressHelper ipv4;
   ipv4.SetBase ("10.1.1.0", "255.255.255.0");
   Ipv4InterfaceContainer iAiE = ipv4.Assign (dAdE);
 
   ipv4.SetBase ("10.1.2.0", "255.255.255.0");
   Ipv4InterfaceContainer iEiG = ipv4.Assign (dEdG);
 
	ipv4.SetBase ("10.1.3.0", "255.255.255.0");
	Ipv4InterfaceContainer iBiF = ipv4.Assign (dBdF);
	
	ipv4.SetBase ("10.1.4.0", "255.255.255.0");
	Ipv4InterfaceContainer iCiF = ipv4.Assign (dCdF);
	
	ipv4.SetBase ("10.1.5.0", "255.255.255.0");
	Ipv4InterfaceContainer iFiG = ipv4.Assign (dFdG);
	
	ipv4.SetBase ("10.1.6.0", "255.255.255.0");
	Ipv4InterfaceContainer iDiG = ipv4.Assign (dDdG);
	
	ipv4.SetBase ("10.1.7.0", "255.255.255.0");
	Ipv4InterfaceContainer iGiR = ipv4.Assign (dGdR);
	
	ipv4.SetBase ("10.1.8.0", "255.255.255.0");
	Ipv4InterfaceContainer iGiS = ipv4.Assign (dGdS);
 
   // Create router nodes, initialize routing database and set up the routing
   // tables in the nodes.
   Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
   
   
     NS_LOG_INFO ("Create Applications.");
	//
	// Create a UdpEchoServer application on node one.
	//
   uint16_t port = 9;  // well-known echo port number
   UdpEchoServerHelper server (port);
   ApplicationContainer apps = server.Install (c.Get (3));
   apps.Start (Seconds (1.0));
   apps.Stop (Seconds (10.0));
   

//
 // Create a UdpEchoClient application to send UDP datagrams from node zero to
 // node one.
 //
 
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
 

 
   AsciiTraceHelper ascii;
   p2p.EnableAsciiAll (ascii.CreateFileStream ("simple-global-routing.tr"));
   p2p.EnablePcapAll ("simple-global-routing");
 
   // Flow Monitor
   FlowMonitorHelper flowmonHelper;
   if (enableFlowMonitor)
     {
       flowmonHelper.InstallAll ();
     }
 
   NS_LOG_INFO ("Run Simulation.");
   Simulator::Stop (Seconds (11));
   Simulator::Run ();
   NS_LOG_INFO ("Done.");
 
   if (enableFlowMonitor)
     {
       flowmonHelper.SerializeToXmlFile ("simple-global-routing.flowmon", false, false);
     }
 
   Simulator::Destroy ();
   return 0;
 }