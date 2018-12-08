/*
  Network topology:
  A----R----B 
*/

#include <iostream>
#include <fstream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

std::string fileNameRoot = "project";    

void CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << " " <<  newCwnd << std::endl;
}

static void
TraceCwnd ()   
{
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream (fileNameRoot + ".cwnd");
  Config::ConnectWithoutContext ("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeBoundCallback (&CwndChange,stream));
}


void
ChangeDelay(int delay){
  //P.SetChannelAttribute("Delay", TimeValue (MilliSeconds(delay)));
  Config::Set("/ChannelList/1/$ns3::PointToPointChannel/Delay",TimeValue (MilliSeconds(delay)));
  Config::Set("/ChannelList/1/$ns3::PointToPointChannel/Delay",TimeValue (MilliSeconds(delay)));
  std::cout << "test" << std::endl;
}

int main (int argc, char *argv[])
{
  int tcpSegmentSize = 1000;
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (tcpSegmentSize));
  Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (0));
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpVegas"));
  //Config::SetDefault ("ns3::RttEstimator::MinRTO", TimeValue (MilliSeconds (500)));

  unsigned int runtime = 20;   
  int delayAR = 10;            
  int delayRB = 134;           
  double bottleneckBW= 10;   
  double fastBW = 1.5;          
  uint32_t queuesize = 7; 
  uint32_t maxBytes = 0;      
  CommandLine cmd;
  

  cmd.AddValue ("runtime", "How long the applications should send data", runtime);
  cmd.AddValue ("delayRB", "Delay on the R--B link, in ms", delayRB);
  cmd.AddValue ("queuesize", "queue size at R", queuesize);
  cmd.AddValue ("tcpSegmentSize", "TCP segment size", tcpSegmentSize);
  
  cmd.Parse (argc, argv);

  std::cout << "queuesize=" << queuesize << ", delayRB=" << delayRB << std::endl;
  Ptr<Node> A = CreateObject<Node> ();
  Ptr<Node> R = CreateObject<Node> ();
  Ptr<Node> B = CreateObject<Node> ();
         
  NetDeviceContainer devAR, devRB;
  PointToPointHelper AR, RB;

  // link from A to R
  AR.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (fastBW * 1000 * 1000)));
  AR.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (delayAR)));
  devAR = AR.Install(A, R);

  RB.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (bottleneckBW * 1000 * 1000)));
  RB.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (delayRB)));
  RB.SetQueue("ns3::DropTailQueue","MaxSize", StringValue("7p")); 
  //RB.SetQueue("ns3::DropTailQueue", "MaxPackets", UintegerValue(queuesize));
  devRB = RB.Install(R,B);

  InternetStackHelper internet;
  internet.Install (A);
  internet.Install (R);
  internet.Install (B);

  // ip addresses

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer ipv4Interfaces;
  ipv4Interfaces.Add(ipv4.Assign (devAR));
  ipv4.SetBase ("10.0.1.0", "255.255.255.0");
  ipv4Interfaces.Add(ipv4.Assign(devRB));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Ptr<Ipv4> A4 = A->GetObject<Ipv4>();  
  Ptr<Ipv4> B4 = B->GetObject<Ipv4>();
  Ptr<Ipv4> R4 = R->GetObject<Ipv4>();
  Ipv4Address Aaddr = A4->GetAddress(1,0).GetLocal();
  Ipv4Address Baddr = B4->GetAddress(1,0).GetLocal();
  Ipv4Address Raddr = R4->GetAddress(1,0).GetLocal();

  std::cout << "A's address: " << Aaddr << std::endl;
  std::cout << "B's address: " << Baddr << std::endl;
  std::cout << "R's #1 address: " << Raddr << std::endl;
  std::cout << "R's #2 address: " << R4->GetAddress(2,0).GetLocal() << std::endl;

  // sink 
  uint16_t Bport = 80;
  Address sinkAaddr(InetSocketAddress (Ipv4Address::GetAny (), Bport));
  PacketSinkHelper sinkA ("ns3::TcpSocketFactory", sinkAaddr);
  ApplicationContainer sinkAppA = sinkA.Install (B);
  sinkAppA.Start (Seconds (0.01));

  sinkAppA.Stop (Seconds (runtime + 60.0));

  Address sinkAddr(InetSocketAddress(Baddr, Bport));

#ifdef USE_HELPER

  BulkSendHelper sourceAhelper ("ns3::TcpSocketFactory",  sinkAddr);
  sourceAhelper.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  sourceAhelper.SetAttribute ("SendSize", UintegerValue (tcpSegmentSize));
  ApplicationContainer sourceAppsA = sourceAhelper.Install (A);
  sourceAppsA.Start (Seconds (0.0));
  sourceAppsA.Stop (Seconds (runtime));

#else

  ObjectFactory factory;
  factory.SetTypeId ("ns3::BulkSendApplication");
  factory.Set ("Protocol", StringValue ("ns3::TcpSocketFactory"));
  factory.Set ("MaxBytes", UintegerValue (maxBytes));
  factory.Set ("SendSize", UintegerValue (tcpSegmentSize));
  factory.Set ("Remote", AddressValue (sinkAddr));
  Ptr<Object> bulkSendAppObj = factory.Create();
  Ptr<Application> bulkSendApp = bulkSendAppObj -> GetObject<Application>();
  bulkSendApp->SetStartTime(Seconds(0.0));
  bulkSendApp->SetStopTime(Seconds(runtime));
  A->AddApplication(bulkSendApp);

#endif

  AsciiTraceHelper ascii;
  std::string tfname = fileNameRoot + ".tr";
  AR.EnableAsciiAll (ascii.CreateFileStream (tfname));
  Simulator::Schedule(Seconds(0.01),&TraceCwnd);      
  

  Simulator::Schedule(Seconds(10.0),&ChangeDelay, 400);

  Simulator::Schedule(Seconds(15.0),&ChangeDelay, 88);

  AR.EnablePcapAll (fileNameRoot);  

  Simulator::Stop (Seconds (runtime+60));
  Simulator::Run ();

  Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkAppA.Get (0));
  std::cout << "Total Bytes Received from A: " << sink1->GetTotalRx () << std::endl;
  return 0;
}
