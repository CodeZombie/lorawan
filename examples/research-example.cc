/*
 * This script simulates a simple network in which one end device sends one
 * packet to the gateway.
 */

#include "ns3/end-device-lora-phy.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/end-device-lorawan-mac.h"
#include "ns3/gateway-lorawan-mac.h"
#include "ns3/periodic-sender-helper.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/lora-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/node-container.h"
#include "ns3/position-allocator.h"
#include "ns3/one-shot-sender-helper.h"
#include "ns3/network-server-helper.h"
#include "ns3/forwarder-helper.h"

#include "ns3/trace-print-helper.h"

#include "ns3/command-line.h"
#include <algorithm>
#include <ctime>

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE ("SimpleLorawanNetworkExample");

/*******************************
             CONFIG            *
 ******************************/
const bool UseGeneticAlgorithm = false;     //wether the MAC should use Genetic Algorithms or ADR.
Time simTime = Hours (25);                  //How long the simulation should run for.
int NumberOfNodes = 4;                      //The number of end-node devices in the network.
Time transmitInterval = Hours(0);           //How frequently end-nodes transmit. 0 = Random.
Time dataCaptureInterval = Hours(1);        //The time in between data sampling.
std::string adrType = "ns3::AdrComponent";  //????????

int main (int argc, char *argv[])
{
  //open trace files:
  // Set up logging
  //LogComponentEnable ("SimpleLorawanNetworkExample", LOG_LEVEL_ALL);
  //LogComponentEnable ("LoraChannel", LOG_LEVEL_INFO);
  //LogComponentEnable ("LoraPhy", LOG_LEVEL_ALL);
  //LogComponentEnable ("EndDeviceLoraPhy", LOG_LEVEL_ALL);
  //LogComponentEnable ("GatewayLoraPhy", LOG_LEVEL_ALL);
  //LogComponentEnable ("LoraInterferenceHelper", LOG_LEVEL_ALL);
  //LogComponentEnable ("LorawanMac", LOG_LEVEL_ALL);
  //LogComponentEnable ("EndDeviceLorawanMac", LOG_LEVEL_ALL);
  //LogComponentEnable ("ClassAEndDeviceLorawanMac", LOG_LEVEL_ALL);
  //LogComponentEnable ("GatewayLorawanMac", LOG_LEVEL_ALL);
  //LogComponentEnable ("LogicalLoraChannelHelper", LOG_LEVEL_ALL);
  //LogComponentEnable ("LogicalLoraChannel", LOG_LEVEL_ALL);
  //LogComponentEnable ("LoraHelper", LOG_LEVEL_ALL);
  //LogComponentEnable ("LoraPhyHelper", LOG_LEVEL_ALL);
  //LogComponentEnable ("LorawanMacHelper", LOG_LEVEL_ALL);
  //LogComponentEnable ("OneShotSenderHelper", LOG_LEVEL_ALL);
  //LogComponentEnable ("OneShotSender", LOG_LEVEL_ALL);
  //LogComponentEnable ("LorawanMacHeader", LOG_LEVEL_ALL);
  //LogComponentEnable ("LoraFrameHeader", LOG_LEVEL_ALL);
  //LogComponentEnable ("LoraPacketTracker", LOG_LEVEL_ALL);
  
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  LogComponentEnableAll (LOG_PREFIX_TIME);

  if(UseGeneticAlgorithm == false){ 
    Config::SetDefault ("ns3::EndDeviceLorawanMac::DRControl", BooleanValue (true));
  }

  /************************
  *  Create the channel  *
  ************************/

  // Create the lora channel object
  Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel> ();
  loss->SetPathLossExponent (3.76);
  loss->SetReference (1, 7.7);
  Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel> ();
  Ptr<LoraChannel> channel = CreateObject<LoraChannel> (loss, delay);

  /************************
  *  Create the helpers  *
  ************************/

  NS_LOG_INFO ("Setting up helpers...");

  TracePrintHelper* tracePrintHelper;

  MobilityHelper mobility;
  /*Ptr<ListPositionAllocator> allocator = CreateObject<ListPositionAllocator> ();
  allocator->Add (Vector (1000,0,0));
  allocator->Add (Vector (0,0,0));
  mobility.SetPositionAllocator (allocator);*/
  //2650
  mobility.SetPositionAllocator("ns3::UniformDiscPositionAllocator", "rho", DoubleValue (100), "X", DoubleValue (0.0), "Y", DoubleValue (0.0));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // Create the LoraPhyHelper
  LoraPhyHelper phyHelper = LoraPhyHelper ();
  phyHelper.SetChannel (channel);

  // Create the LorawanMacHelper
  LorawanMacHelper macHelper = LorawanMacHelper ();

  // Create the LoraHelper
  LoraHelper helper = LoraHelper ();
  helper.EnablePacketTracking (); // Output filename

  //Create the NetworkServerHelper
  NetworkServerHelper nsHelper = NetworkServerHelper ();
  nsHelper.EnableAdr(!UseGeneticAlgorithm);
  if(!UseGeneticAlgorithm) {
    nsHelper.SetAdr(adrType);
  }
  
  //Create the ForwarderHelper
  ForwarderHelper forHelper = ForwarderHelper ();

  /************************
  *  Create End Devices  *
  ************************/

  // Create a set of nodes
  NodeContainer endDevices;
  
  endDevices.Create (NumberOfNodes);

  // Assign a mobility model to the node
  mobility.Install (endDevices);

  // Create the LoraNetDevices of the end devicesS
  phyHelper.SetDeviceType (LoraPhyHelper::ED);
  macHelper.SetDeviceType (LorawanMacHelper::ED_A);

  //TODO: Our genetic algo should eventually turn confirmation off.
  macHelper.Set("MType", EnumValue(LorawanMacHeader::CONFIRMED_DATA_UP));

  //Enable/Disable the genetic algorithm within the MAC layer.
  macHelper.Set("UseGeneticAlgorithm", BooleanValue(UseGeneticAlgorithm));

  helper.Install (phyHelper, macHelper, endDevices);

  if(UseGeneticAlgorithm) {
    tracePrintHelper = new TracePrintHelper("GA_", &endDevices, dataCaptureInterval);
  }else{
    tracePrintHelper = new TracePrintHelper("ADR_", &endDevices, dataCaptureInterval);
  }
  tracePrintHelper->WatchAttribute("FailedTransmissionCount", TracePrintAttributeTypes::Integer, false);
  tracePrintHelper->WatchAttribute("DataRate", TracePrintAttributeTypes::Uinteger, false);
  tracePrintHelper->WatchAttribute("UseGeneticAlgorithm", TracePrintAttributeTypes::Boolean, false);


  //print end-device locations:
  std::ofstream locationFile;
  locationFile.open ("endDeviceLocations.txt");
  for (NodeContainer::Iterator j = endDevices.Begin (); j != endDevices.End (); ++j)
    {
      Ptr<MobilityModel> mobility = (*j)->GetObject<MobilityModel> ();
      Vector position = mobility->GetPosition ();
      locationFile << position.x << " " << position.y << std::endl;
      
    }

  /*********************
  *  Create Gateways  *
  *********************/

  NS_LOG_INFO ("Creating the gateway...");
  NodeContainer gateways;
  gateways.Create (1);

  mobility.Install (gateways);

  // Create a netdevice for each gateway
  phyHelper.SetDeviceType (LoraPhyHelper::GW);
  macHelper.SetDeviceType (LorawanMacHelper::GW);
  helper.Install (phyHelper, macHelper, gateways);

    //print end-device locations:
  std::ofstream gwlocationFile;
  gwlocationFile.open ("gatewayLocations.txt");
  for (NodeContainer::Iterator j = gateways.Begin (); j != gateways.End (); ++j)
    {
      Ptr<MobilityModel> mobility = (*j)->GetObject<MobilityModel> ();
      Vector position = mobility->GetPosition ();
      gwlocationFile << position.x << " " << position.y << std::endl;
    }


///////////// Enable periodic phy output to file
//helper.EnablePeriodicPhyPerformancePrinting(gateways, "PeriodicPhyPerformance.txt", Seconds(60));

  /*********************************************
  *  Install applications on the end devices  *
  *********************************************/
/*
  OneShotSenderHelper oneShotSenderHelper;
  oneShotSenderHelper.SetSendTime (Seconds (2));

  oneShotSenderHelper.Install (endDevices);
*/

  PeriodicSenderHelper appHelper = PeriodicSenderHelper ();
  appHelper.SetPeriod (transmitInterval);
  appHelper.SetPacketSize (23);
  Ptr<RandomVariableStream> rv = CreateObjectWithAttributes<UniformRandomVariable> ("Min", DoubleValue (0), "Max", DoubleValue (10));
  ApplicationContainer appContainer = appHelper.Install (endDevices);
  appContainer.Start (Seconds (0));
  appContainer.Stop (simTime);
  /******************
   * Set Data Rates *
   ******************/
  //std::vector<int> sfQuantity (6);
  //sfQuantity = macHelper.SetSpreadingFactorsUp (endDevices, gateways, channel);
  

  /**************************
   *  Create Network Server  *
   ***************************/

  // Create the NS node
  NodeContainer networkServer;
  networkServer.Create (1);

  // Create a NS for the network
  nsHelper.SetEndDevices (endDevices);
  nsHelper.SetGateways (gateways);
  nsHelper.Install (networkServer);

  //Create a forwarder for each gateway
  forHelper.Install (gateways);

  /****************
  *  Simulation  *
  ****************/
  
  Simulator::Stop (simTime);

  //Simulator::Schedule(Hours(0.25), &intervalMethod, &endDevices);

  Simulator::Run ();
  //lper.DoPrintDeviceStatus (endDevices, gateways, "DeviceStatus_COOL.txt");
  Simulator::Destroy ();

  LoraPacketTracker &tracker = helper.GetPacketTracker ();
  std::cout << tracker.CountMacPacketsGlobally(Seconds(0), simTime + Hours(1)) << std::endl;
  std::cout << tracker.CountMacPacketsGloballyCpsr(Seconds(0), simTime + Hours(1)) << std::endl;


  return 0;
}