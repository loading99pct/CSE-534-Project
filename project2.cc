

#include <iostream>
#include <fstream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

// #define USE_HELPER

using namespace ns3;

std::string fileNameRoot = "project2";    

void CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << " " <<  newCwnd << std::endl;
}

static void
TraceCwnd ()    
{
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream (fileNameRoot + "0.cwnd");
  Config::ConnectWithoutContext ("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeBoundCallback (&CwndChange,stream));

  Ptr<OutputStreamWrapper> stream1 = ascii.CreateFileStream (fileNameRoot + "1.cwnd");
  Config::ConnectWithoutContext ("/NodeList/1/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeBoundCallback (&CwndChange,stream1));

  Ptr<OutputStreamWrapper> stream2 = ascii.CreateFileStream (fileNameRoot + "2.cwnd");
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeBoundCallback (&CwndChange,stream2));

  Ptr<OutputStreamWrapper> stream3 = ascii.CreateFileStream (fileNameRoot + "3.cwnd");
  Config::ConnectWithoutContext ("/NodeList/3/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeBoundCallback (&CwndChange,stream3));

  Ptr<OutputStreamWrapper> stream4 = ascii.CreateFileStream (fileNameRoot + "4.cwnd");
  Config::ConnectWithoutContext ("/NodeList/4/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeBoundCallback (&CwndChange,stream4));

}



void
ChangeDelay(int delay){

  Config::Set("/ChannelList/5/$ns3::PointToPointChannel/Delay",TimeValue (MilliSeconds(delay)));
  std::cout << "test" << std::endl;
}

int main (int argc, char *argv[])
{
  int tcpSegmentSize = 2000;
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (tcpSegmentSize));
  Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (0));
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpVegas"));
  //Config::SetDefault ("ns3::RttEstimator::MinRTO", TimeValue (MilliSeconds (500)));

  unsigned int runtime = 50;   
  int delayAR = 60;           
  int delayA1R = 52;
  int delayA2R = 48;
  int delayA3R = 64;
  int delayA4R = 56;
  int delayRB = 10;           
  double bottleneckBW= 10;  
  double fastBW = 3.5;         
  uint32_t queuesize = 7; 
  uint32_t maxBytes = 0;       
  float interval = 5.0;

  CommandLine cmd;

  cmd.AddValue ("runtime", "How long the applications should send data", runtime);
  cmd.AddValue ("delayRB", "Delay on the R--B link, in ms", delayRB);
  cmd.AddValue ("queuesize", "queue size at R", queuesize);
  cmd.AddValue ("tcpSegmentSize", "TCP segment size", tcpSegmentSize);
  
  cmd.Parse (argc, argv);

  std::cout << "queuesize=" << queuesize << ", delayRB=" << delayRB << std::endl;
  Ptr<Node> A = CreateObject<Node> ();
  Ptr<Node> An1 = CreateObject<Node> ();
  Ptr<Node> An2 = CreateObject<Node> ();
  Ptr<Node> An3 = CreateObject<Node> ();
  Ptr<Node> An4 = CreateObject<Node> ();
  Ptr<Node> R = CreateObject<Node> ();
  Ptr<Node> B = CreateObject<Node> ();
         
  NetDeviceContainer devAR, devRB, devA1R, devA2R, devA3R, devA4R;
  PointToPointHelper AR, RB, A1R, A2R, A3R, A4R;

  // create point-to-point link from A to R
  AR.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (bottleneckBW * 1000 * 1000)));
  AR.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (delayAR)));
  devAR = AR.Install(A, R);

  // create point-to-point link from A1 to R
  A1R.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (bottleneckBW * 1000 * 1000)));
  A1R.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (delayA1R)));
  devA1R = A1R.Install(An1, R);

  // create point-to-point link from A2 to R
  A2R.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (bottleneckBW * 1000 * 1000)));
  A2R.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (delayA2R)));
  devA2R = A2R.Install(An2, R);
  // create point-to-point link from A3 to R
  A3R.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (bottleneckBW * 1000 * 1000)));
  A3R.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (delayA3R)));
  devA3R = A3R.Install(An3, R);

  // create point-to-point link from A4 to R
  A4R.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (bottleneckBW * 1000 * 1000)));
  A4R.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (delayA4R)));
  devA4R = A4R.Install(An4, R);
  // create point-to-point link from R to B
  RB.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (fastBW * 1000 * 1000)));
  RB.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (delayRB)));
  RB.SetQueue("ns3::DropTailQueue","MaxSize", StringValue("30p")); 
  //RB.SetQueue("ns3::DropTailQueue", "MaxPackets", UintegerValue(queuesize));
  devRB = RB.Install(R,B);

  InternetStackHelper internet;
  internet.Install (A);
  internet.Install (An1);
  internet.Install (An2);
  internet.Install (An3);
  internet.Install (An4);
  internet.Install (R);
  internet.Install (B);


  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer ipv4Interfaces;
  ipv4Interfaces.Add(ipv4.Assign (devAR));
  ipv4.SetBase ("10.0.2.0", "255.255.255.0");
  ipv4Interfaces.Add(ipv4.Assign (devA1R));
  ipv4.SetBase ("10.0.3.0", "255.255.255.0");
  ipv4Interfaces.Add(ipv4.Assign (devA2R));
  ipv4.SetBase ("10.0.4.0", "255.255.255.0");
  ipv4Interfaces.Add(ipv4.Assign (devA3R));
  ipv4.SetBase ("10.0.5.0", "255.255.255.0");
  ipv4Interfaces.Add(ipv4.Assign (devA4R));
  ipv4.SetBase ("10.0.1.0", "255.255.255.0");
  ipv4Interfaces.Add(ipv4.Assign(devRB));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Ptr<Ipv4> A4 = A->GetObject<Ipv4>(); 
  Ptr<Ipv4> A14 = An1->GetObject<Ipv4>();  
  Ptr<Ipv4> A24 = An2->GetObject<Ipv4>();    
  Ptr<Ipv4> A34 = An3->GetObject<Ipv4>();  
  Ptr<Ipv4> A44 = An4->GetObject<Ipv4>();   
  Ptr<Ipv4> B4 = B->GetObject<Ipv4>();
  Ptr<Ipv4> R4 = R->GetObject<Ipv4>();
  Ipv4Address Aaddr = A4->GetAddress(1,0).GetLocal();
  Ipv4Address A1addr = A14->GetAddress(1,0).GetLocal();
  Ipv4Address A2addr = A24->GetAddress(1,0).GetLocal();
  Ipv4Address A3addr = A34->GetAddress(1,0).GetLocal();
  Ipv4Address A4addr = A44->GetAddress(1,0).GetLocal();
  Ipv4Address Baddr = B4->GetAddress(1,0).GetLocal();
  Ipv4Address Raddr = R4->GetAddress(1,0).GetLocal();

  std::cout << "A's address: " << Aaddr << std::endl;
  std::cout << "A1's address: " << A1addr << std::endl;
  std::cout << "A2's address: " << A2addr << std::endl;
  std::cout << "A3's address: " << A3addr << std::endl;
  std::cout << "A4's address: " << A4addr << std::endl;
  std::cout << "B's address: " << Baddr << std::endl;
  std::cout << "R's #1 address: " << Raddr << std::endl;
  std::cout << "R's #2 address: " << R4->GetAddress(2,0).GetLocal() << std::endl;


  uint16_t Bport = 80;
  std::cout << "Ipv4Address::GetAny (): " << Ipv4Address::GetAny () << std::endl;
  // Address sinkAaddr(InetSocketAddress (Ipv4Address::GetAny (), Bport));
  Address sinkAaddr(InetSocketAddress (Baddr, Bport));
  std::cout << "sinkAaddr: " << sinkAaddr << std::endl;
  PacketSinkHelper sinkA ("ns3::TcpSocketFactory", sinkAaddr);
  ApplicationContainer sinkAppA = sinkA.Install (B);
  sinkAppA.Start (Seconds (0.01));

  sinkAppA.Stop (Seconds (runtime + 60.0));

  Address sinkAddr(InetSocketAddress(Baddr, Bport));


  uint16_t Bport1 = 81;
  // std::cout << "Ipv4Address::GetAny (): " << Ipv4Address::GetAny () << std::endl;
  // Address sinkAaddr(InetSocketAddress (Ipv4Address::GetAny (), Bport));
  Address sinkA1addr(InetSocketAddress (Baddr, Bport1));
  std::cout << "sinkA1addr: " << sinkA1addr << std::endl;
  PacketSinkHelper sinkA1 ("ns3::TcpSocketFactory", sinkA1addr);
  ApplicationContainer sinkAppA1 = sinkA1.Install (B);
  sinkAppA1.Start (Seconds (interval + 0.01));

  sinkAppA1.Stop (Seconds (runtime + 60.0));

  Address sinkAddr1(InetSocketAddress(Baddr, Bport1));


  uint16_t Bport2 = 82;
  // std::cout << "Ipv4Address::GetAny (): " << Ipv4Address::GetAny () << std::endl;
  // Address sinkAaddr(InetSocketAddress (Ipv4Address::GetAny (), Bport));
  Address sinkA2addr(InetSocketAddress (Baddr, Bport2));
  std::cout << "sinkA2addr: " << sinkA2addr << std::endl;
  PacketSinkHelper sinkA2 ("ns3::TcpSocketFactory", sinkA2addr);
  ApplicationContainer sinkAppA2 = sinkA2.Install (B);
  sinkAppA2.Start (Seconds (2*interval + 0.01));

  sinkAppA2.Stop (Seconds (runtime + 60.0));

  Address sinkAddr2(InetSocketAddress(Baddr, Bport2));


  uint16_t Bport3 = 83;
  // std::cout << "Ipv4Address::GetAny (): " << Ipv4Address::GetAny () << std::endl;
  // Address sinkAaddr(InetSocketAddress (Ipv4Address::GetAny (), Bport));
  Address sinkA3addr(InetSocketAddress (Baddr, Bport3));
  std::cout << "sinkA3addr: " << sinkA3addr << std::endl;
  PacketSinkHelper sinkA3 ("ns3::TcpSocketFactory", sinkA3addr);
  ApplicationContainer sinkAppA3 = sinkA3.Install (B);
  sinkAppA3.Start (Seconds (3*interval + 0.01));

  sinkAppA3.Stop (Seconds (runtime + 60.0));

  Address sinkAddr3(InetSocketAddress(Baddr, Bport3));

  // create a sink on B
  uint16_t Bport4 = 84;
  // std::cout << "Ipv4Address::GetAny (): " << Ipv4Address::GetAny () << std::endl;
  // Address sinkAaddr(InetSocketAddress (Ipv4Address::GetAny (), Bport));
  Address sinkA4addr(InetSocketAddress (Baddr, Bport4));
  std::cout << "sinkA4addr: " << sinkA4addr << std::endl;
  PacketSinkHelper sinkA4 ("ns3::TcpSocketFactory", sinkA4addr);
  ApplicationContainer sinkAppA4 = sinkA4.Install (B);
  sinkAppA4.Start (Seconds (4*interval + 0.01));

  sinkAppA4.Stop (Seconds (runtime + 60.0));

  Address sinkAddr4(InetSocketAddress(Baddr, Bport4));


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

  // ObjectFactory factory;
  factory.SetTypeId ("ns3::BulkSendApplication");
  factory.Set ("Protocol", StringValue ("ns3::TcpSocketFactory"));
  factory.Set ("MaxBytes", UintegerValue (maxBytes));
  factory.Set ("SendSize", UintegerValue (tcpSegmentSize));
  factory.Set ("Remote", AddressValue (sinkAddr1));
  Ptr<Object> bulkSendAppObj1 = factory.Create();
  Ptr<Application> bulkSendApp1 = bulkSendAppObj1 -> GetObject<Application>();
  bulkSendApp1->SetStartTime(Seconds(0.0));
  bulkSendApp1->SetStopTime(Seconds(runtime));
  An1->AddApplication(bulkSendApp1);

  // ObjectFactory factory;
  factory.SetTypeId ("ns3::BulkSendApplication");
  factory.Set ("Protocol", StringValue ("ns3::TcpSocketFactory"));
  factory.Set ("MaxBytes", UintegerValue (maxBytes));
  factory.Set ("SendSize", UintegerValue (tcpSegmentSize));
  factory.Set ("Remote", AddressValue (sinkAddr2));
  Ptr<Object> bulkSendAppObj2 = factory.Create();
  Ptr<Application> bulkSendApp2 = bulkSendAppObj2 -> GetObject<Application>();
  bulkSendApp2->SetStartTime(Seconds(0.0));
  bulkSendApp2->SetStopTime(Seconds(runtime));
  An2->AddApplication(bulkSendApp2);

  // ObjectFactory factory;
  factory.SetTypeId ("ns3::BulkSendApplication");
  factory.Set ("Protocol", StringValue ("ns3::TcpSocketFactory"));
  factory.Set ("MaxBytes", UintegerValue (maxBytes));
  factory.Set ("SendSize", UintegerValue (tcpSegmentSize));
  factory.Set ("Remote", AddressValue (sinkAddr3));
  Ptr<Object> bulkSendAppObj3 = factory.Create();
  Ptr<Application> bulkSendApp3 = bulkSendAppObj3 -> GetObject<Application>();
  bulkSendApp3->SetStartTime(Seconds(0.0));
  bulkSendApp3->SetStopTime(Seconds(runtime));
  An3->AddApplication(bulkSendApp3);

  // ObjectFactory factory;
  factory.SetTypeId ("ns3::BulkSendApplication");
  factory.Set ("Protocol", StringValue ("ns3::TcpSocketFactory"));
  factory.Set ("MaxBytes", UintegerValue (maxBytes));
  factory.Set ("SendSize", UintegerValue (tcpSegmentSize));
  factory.Set ("Remote", AddressValue (sinkAddr4));
  Ptr<Object> bulkSendAppObj4 = factory.Create();
  Ptr<Application> bulkSendApp4 = bulkSendAppObj4 -> GetObject<Application>();
  bulkSendApp4->SetStartTime(Seconds(0.0));
  bulkSendApp4->SetStopTime(Seconds(runtime));
  An4->AddApplication(bulkSendApp4);
#endif


  AsciiTraceHelper ascii;
  std::string tfname = fileNameRoot + ".tr";
  AR.EnableAsciiAll (ascii.CreateFileStream (tfname));

  Simulator::Schedule(Seconds(0.01),&TraceCwnd);       // this Time cannot be 0.0
  

  //Simulator::Schedule(Seconds(10.0),&ChangeDelay, 400);

  //Simulator::Schedule(Seconds(15.0),&ChangeDelay, 88);

  AR.EnablePcapAll (fileNameRoot);    

  Simulator::Stop (Seconds (runtime+60));
  Simulator::Run ();

  Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkAppA.Get (0));
  std::cout << "Total Bytes Received from A: " << sink1->GetTotalRx () << std::endl;

  Ptr<PacketSink> sink2 = DynamicCast<PacketSink> (sinkAppA1.Get (0));
  std::cout << "Total Bytes Received from A1: " << sink2->GetTotalRx () << std::endl;

  Ptr<PacketSink> sink3 = DynamicCast<PacketSink> (sinkAppA2.Get (0));
  std::cout << "Total Bytes Received from A2: " << sink3->GetTotalRx () << std::endl;

  Ptr<PacketSink> sink4 = DynamicCast<PacketSink> (sinkAppA3.Get (0));
  std::cout << "Total Bytes Received from A3: " << sink4->GetTotalRx () << std::endl;

  Ptr<PacketSink> sink5 = DynamicCast<PacketSink> (sinkAppA4.Get (0));
  std::cout << "Total Bytes Received from A4: " << sink5->GetTotalRx () << std::endl;
  return 0;
}
