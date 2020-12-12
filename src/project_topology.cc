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
 
 int main(int argc, char *argv[]) {
    // ########################################################################
    // Main routine
    // ########################################################################
    NS_LOG_COMPONENT_DEFINE ("P2PToplpgyNetwork");

    std::string dataRate5      = "5Mbps";
    std::string dataRate8      = "8Mbps";
    std::string dataRate10     = "10Mbps";

    // ======================================================================
    // Create the nodes & links required for the topology shown in comments above.
    // ----------------------------------------------------------------------
    // NS_LOG_INFO ("INFO: Create nodes.");

    Ptr<Node> A  = CreateObject<Node> ();
    Ptr<Node> B  = CreateObject<Node> ();
    Ptr<Node> C  = CreateObject<Node> ();
    Ptr<Node> D  = CreateObject<Node> ();

    Ptr<Node> E  = CreateObject<Node> ();
    Ptr<Node> F  = CreateObject<Node> ();
    Ptr<Node> G  = CreateObject<Node> ();

    Ptr<Node> SERVER  = CreateObject<Node> ();
    Ptr<Node> ROUTER  = CreateObject<Node> ();

    // ======================================================================
    // Define the P2P Helpers
    // ----------------------------------------------------------------------

    PointToPointHelper p2p_5;
    p2p_5.SetDeviceAttribute  ("DataRate", StringValue (dataRate5));

    PointToPointHelper p2p_8;
    p2p_8.SetDeviceAttribute  ("DataRate", StringValue (dataRate8));

    PointToPointHelper p2p_10;
    p2p_10.SetDeviceAttribute  ("DataRate", StringValue (dataRate10));

    PointToPointHelper p2p_GS;
    p2p_GS.SetDeviceAttribute  ("DataRate", StringValue (dataRate10));

    // ======================================================================
    // Define the P2P Helpers
    // ----------------------------------------------------------------------

    NodeContainer nc_A_E = NodeContainer (A,E);
    NetDeviceContainer link_A_E;
    link_A_E = p2p_5.Install (nc_A_E);

    NodeContainer nc_B_F = NodeContainer (B,F);
    NetDeviceContainer link_B_F;
    link_B_F = p2p_5.Install (nc_B_F);

    NodeContainer nc_C_F = NodeContainer (C,F);
    NetDeviceContainer link_C_F;
    link_C_F = p2p_5.Install (nc_C_F);

    NodeContainer nc_D_G = NodeContainer (D,G);
    NetDeviceContainer link_D_G;
    link_D_G = p2p_5.Install (nc_D_G);

    NodeContainer nc_E_G = NodeContainer (E,G);
    NetDeviceContainer link_E_G;
    link_E_G = p2p_5.Install (nc_E_G);

    NodeContainer nc_F_G = NodeContainer (F,G);
    NetDeviceContainer link_F_G;
    link_F_G = p2p_8.Install (nc_F_G);

    NodeContainer nc_G_S = NodeContainer (G,SERVER);
    NetDeviceContainer link_G_S;
    link_G_S = p2p_GS.Install (nc_G_S);

    NodeContainer nc_G_R = NodeContainer (G,ROUTER);
    NetDeviceContainer link_G_R;
    link_G_R = p2p_10.Install (nc_G_R);

    InternetStackHelper stack;
    stack.Install(A);
    stack.Install(B);
    stack.Install(C);
    stack.Install(D);

    stack.Install(E);
    stack.Install(F);
    stack.Install(G);

    stack.Install(SERVER);
    stack.Install(ROUTER);
      
    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces_A_E = address.Assign(link_A_E);

    address.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces_B_F = address.Assign(link_B_F);

    address.SetBase ("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces_C_F = address.Assign(link_C_F);

    address.SetBase ("10.1.4.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces_D_G = address.Assign(link_D_G);

    address.SetBase ("10.1.5.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces_E_G = address.Assign(link_E_G);

    address.SetBase ("10.1.6.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces_F_G = address.Assign(link_F_G);

    address.SetBase ("10.1.7.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces_G_S = address.Assign(link_G_S);

    address.SetBase ("10.1.8.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces_G_R = address.Assign(link_G_R);


    UdpEchoClientHelper echoClient (interfaces_G_S.GetAddress (1), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

    ApplicationContainer clientApps = echoClient.Install (nc_A_E.Get (0));
    clientApps.Start (Seconds (2.0));
    clientApps.Stop (Seconds (10.0));

    UdpEchoServerHelper echoServer (9);
    ApplicationContainer serverApps = echoServer.Install (nc_G_S.Get (1));
    serverApps.Start (Seconds (1.0));
    serverApps.Stop (Seconds (10.0));

    AsciiTraceHelper ascii;
    p2p_GS.EnableAsciiAll(ascii.CreateFileStream("toplogy.tr"));

    p2p_GS.EnablePcapAll("topology");

    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}