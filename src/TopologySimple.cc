 #include <iostream>
 #include <fstream>
 
 #include "ns3/core-module.h"
 #include "ns3/network-module.h"
 #include "ns3/applications-module.h"
 #include "ns3/bridge-module.h"
 #include "ns3/csma-module.h"
 #include "ns3/point-to-point-module.h"
 #include "ns3/internet-module.h"
 
 using namespace ns3;

 NS_LOG_COMPONENT_DEFINE ("SimplePointToPointExample");

int  main (int argc, char *argv[])
 {
    std::string dataRate5  = "5Mbps";

    PointToPointHelper p2p_5;
    p2p_5.SetDeviceAttribute  ("DataRate", StringValue (dataRate5));

    NS_LOG_INFO ("Create nodes.");
    NodeContainer c;
    c.Create (4);
    NodeContainer n02 = NodeContainer (c.Get (0), c.Get (2));
    NodeContainer n12 = NodeContainer (c.Get (1), c.Get (2));
    NodeContainer n32 = NodeContainer (c.Get (3), c.Get (2));

    NS_LOG_INFO ("Device installed.");
    NetDeviceContainer nd02 = p2p.Install (n02);
    NetDeviceContainer nd12 = p2p.Install (n12);
    NetDeviceContainer nd32 = p2p.Install (n32);

    NS_LOG_INFO ("Assign IP Addresses.");
    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i02 = ipv4.Assign (nd02);
 
    ipv4.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer i12 = ipv4.Assign (nd12);
 
    ipv4.SetBase ("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer i32 = ipv4.Assign (nd32);

    NS_LOG_INFO ("Configure Server.");
    UdpEchoServerHelper echoServer (9);
    ApplicationContainer serverApps = echoServer.Install (c.Get (3));
    serverApps.Start (Seconds (1.0));
    serverApps.Stop (Seconds (10.0));

    NS_LOG_INFO ("Configure Client.");
    UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
    ApplicationContainer clientApps = echoClient.Install (c.Get (0));
    clientApps.Start (Seconds (2.0));
    clientApps.Stop (Seconds (10.0));


    AsciiTraceHelper ascii;
    p2p.EnableAsciiAll (ascii.CreateFileStream ("simple-p2p.tr"));
    p2p.EnablePcapAll ("simple-p2p");
 
    Simulator::Stop (Seconds (30));
 
    NS_LOG_INFO ("Run Simulation.");
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");
 
    return 0;
 }
