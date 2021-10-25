/*
 * This script simulates a simple network in which one end device sends one
 * packet to the gateway.
 * 
 * TODO:
 * Implement buildings.
 * Print building and node locations to a dat file.
 * Setup the MAC so that it eventaully converges on an optimal setting after x transmissions and stops mutating/crossover.
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

NS_LOG_COMPONENT_DEFINE("SimpleLorawanNetworkExample");

/*******************************
             CONFIG            *
 ******************************/
//TODO: These should be modified by CMD so we can set up automation routines.
//TODO: Add some Genetic Algorithm parameter customization options here. (Population size, mutation rate, etc)

bool UseGeneticAlgorithm = false;          //whether the MAC should use Genetic Algorithms or ADR.
int simTimeHours = 350;                    //How long the simulation should run for.
int NumberOfNodes = 1182;                  //The number of end-node devices in the network.
Time transmitInterval = Hours(0.2);          //How frequently end-nodes transmit. 0 = Random.
Time dataCaptureInterval = Minutes(32);    //The time in between data sampling.
std::string adrType = "ns3::AdrComponent"; //????????
std::string outputFolder = "dat_output";   //Where output files (.dat) will be stored.
double maxRandomLoss = 0;                  //The maximum amount of random loss that can be
                                           //incurred by a transmission.

int main(int argc, char *argv[])
{
  /*****************************
   ****** COMMAND LINE *********
   ****************************/
  CommandLine cmd;
  cmd.AddValue("nodes", "Number of end nodes to include in the simulation", NumberOfNodes);
  cmd.AddValue("genetic", "Whether to use the genetic algorithm or ADR algorithm", UseGeneticAlgorithm);
  cmd.AddValue("randomloss", "The amount of random loss (noise) for signals.", maxRandomLoss);
  cmd.AddValue("simulationtime", "The length of the simulation in Hours", simTimeHours);
  cmd.AddValue("outputfolder", "The name of the folder to stick output data into.", outputFolder);

  //cmd.AddValue("macpopulationsize", "The size of each population in the MAC Layer's Genetic Algorithm", macPopulationSize);
  //cmd.AddValue("macmutationrate", "The mutation rate of individuals in the MAC Layer's Genetic Algorithm", macMutationRate);
  //cmd.AddValue("gateways", "The number of gateways in the network", gatewayCount);
  cmd.Parse(argc, argv);

  /*****************************
   ******** LOGGING ************
   ****************************/
  //LogComponentEnable ("AdrComponent", LOG_LEVEL_ALL);
  //LogComponentEnable("GeneticTransmissionParameterOptimizer", LOG_LEVEL_ALL);

  LogComponentEnable("EndDeviceLorawanMac", LOG_LEVEL_WARN);

  if (UseGeneticAlgorithm == false)
  {
    Config::SetDefault("ns3::EndDeviceLorawanMac::DRControl", BooleanValue(true));
  }

  /************************
  *  Create the channel  *
  ************************/

  // Create the loss model.
  Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel>();
  loss->SetPathLossExponent(3.76);
  loss->SetReference(1, 7.7);

  //Add some random-loss to the model to simulate some real world interference.
  Ptr<UniformRandomVariable> uniformRandom = CreateObject<UniformRandomVariable>();
  uniformRandom->SetAttribute("Min", DoubleValue(0.0));
  uniformRandom->SetAttribute("Max", DoubleValue(maxRandomLoss));
  Ptr<RandomPropagationLossModel> randomLoss = CreateObject<RandomPropagationLossModel>();
  randomLoss->SetAttribute("Variable", PointerValue(uniformRandom));
  loss->SetNext(randomLoss);

  //Create the channel from loss and delay.
  Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel>();
  Ptr<LoraChannel> channel = CreateObject<LoraChannel>(loss, delay);

  /************************
  *  Create the helpers  *
  ************************/

  NS_LOG_INFO("Setting up helpers...");

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> allocator = CreateObject<ListPositionAllocator>();
  allocator->Add("node_locations.dat");
  //allocator->Add (Vector (3000,0,0));
  mobility.SetPositionAllocator(allocator);
  //2650
  //mobility.SetPositionAllocator("ns3::UniformDiscPositionAllocator", "rho", DoubleValue (cityRadius), "X", DoubleValue (0.0), "Y", DoubleValue (0.0));

  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  // Create the LoraPhyHelper
  LoraPhyHelper phyHelper = LoraPhyHelper();
  phyHelper.SetChannel(channel);

  // Create the LorawanMacHelper
  LorawanMacHelper macHelper = LorawanMacHelper();

  // Create the LoraHelper
  LoraHelper helper = LoraHelper();
  helper.EnablePacketTracking(); // Output filename

  //Create the NetworkServerHelper
  NetworkServerHelper nsHelper = NetworkServerHelper();
  nsHelper.EnableAdr(!UseGeneticAlgorithm);
  if (!UseGeneticAlgorithm)
  {
    nsHelper.SetAdr(adrType);
  }

  /************************
  *  Create End Devices  *
  ************************/

  // Create a set of nodes
  NodeContainer endDevices;

  endDevices.Create(NumberOfNodes);

  // Assign a mobility model to the node
  mobility.Install(endDevices);

  // Create the LoraNetDevices of the end devicesS
  phyHelper.SetDeviceType(LoraPhyHelper::ED);
  macHelper.SetDeviceType(LorawanMacHelper::ED_A);

  //TODO: Our genetic algo should eventually turn confirmation off.
  macHelper.Set("MType", EnumValue(LorawanMacHeader::CONFIRMED_DATA_UP));

  if (!UseGeneticAlgorithm)
  {
    macHelper.Set("EnableEDDataRateAdaptation", BooleanValue(true));
  }

  //Enable/Disable the genetic algorithm within the MAC layer.
  macHelper.Set("UseGeneticAlgorithm", BooleanValue(UseGeneticAlgorithm));

  helper.Install(phyHelper, macHelper, endDevices);

  TracePrintHelper *tracePrintHelper;
  tracePrintHelper = new TracePrintHelper(outputFolder + "/", &endDevices, dataCaptureInterval);
  tracePrintHelper->WatchAttribute("FailedTransmissionCount", TracePrintAttributeTypes::Integer, false);
  tracePrintHelper->WatchAttribute("DataRate", TracePrintAttributeTypes::Uinteger, false);
  tracePrintHelper->WatchAttribute("UseGeneticAlgorithm", TracePrintAttributeTypes::Boolean, false);
  tracePrintHelper->WatchAttribute("LastFitnessLevel", TracePrintAttributeTypes::Double, false);
  tracePrintHelper->WatchAttribute("PacketErrorRate", TracePrintAttributeTypes::Double, false);
  tracePrintHelper->WatchAttribute("LastNPSR", TracePrintAttributeTypes::Double, false);

  /******************************
  * Print location of end node(s)
  * *****************************/
  std::ofstream locationFile;
  locationFile.open(outputFolder + "/endDeviceLocations.dat");
  for (NodeContainer::Iterator j = endDevices.Begin(); j != endDevices.End(); ++j)
  {
    Ptr<MobilityModel> mobility = (*j)->GetObject<MobilityModel>();
    Vector position = mobility->GetPosition();
    locationFile << position.x << " " << position.y << std::endl;
  }

  /*********************
  *  Create Gateways  *
  *********************/
  //Create mobility model for the gateway:
  MobilityHelper gatewayMobility;
  Ptr<ListPositionAllocator> gatewayAllocator = CreateObject<ListPositionAllocator>();
  gatewayAllocator->Add(Vector(0, 0, 0)); //repeat this line to add more values to the list.
  gatewayMobility.SetPositionAllocator(gatewayAllocator);

  NodeContainer gateways;
  gateways.Create(1);
  gatewayMobility.Install(gateways);

  // Create a netdevice for each gateway
  phyHelper.SetDeviceType(LoraPhyHelper::GW);
  macHelper.SetDeviceType(LorawanMacHelper::GW);
  helper.Install(phyHelper, macHelper, gateways);

  /**************************
   * Print location of gateway node(s)
   * ************************/
  std::ofstream gwlocationFile;
  gwlocationFile.open(outputFolder + "/gatewayLocations.dat");
  for (NodeContainer::Iterator j = gateways.Begin(); j != gateways.End(); ++j)
  {
    Ptr<MobilityModel> mobility = (*j)->GetObject<MobilityModel>();
    Vector position = mobility->GetPosition();
    gwlocationFile << position.x << " " << position.y << std::endl;
  }

  /*********************************************
  *  Install applications on the end devices  *
  *********************************************/
  PeriodicSenderHelper appHelper = PeriodicSenderHelper();
  appHelper.SetPeriod(transmitInterval);
  appHelper.SetPacketSize(23);
  ApplicationContainer appContainer = appHelper.Install(endDevices);
  appContainer.Start(Seconds(0));
  appContainer.Stop(Hours(simTimeHours));

  /**************************
   *  Create Network Server  *
   ***************************/
  NodeContainer networkServer;
  networkServer.Create(1);
  nsHelper.SetEndDevices(endDevices);
  nsHelper.SetGateways(gateways);
  nsHelper.Install(networkServer);

  ForwarderHelper forHelper = ForwarderHelper();
  forHelper.Install(gateways);

  /****************
  *  Simulation  *
  ****************/
  Simulator::Stop(Hours(simTimeHours));
  Simulator::Run();
  Simulator::Destroy();
  LoraPacketTracker &tracker = helper.GetPacketTracker();
  std::cout << tracker.CountMacPacketsGlobally(Seconds(0), Hours(simTimeHours) + Hours(1)) << std::endl;
  std::cout << tracker.CountMacPacketsGloballyCpsr(Seconds(0), Hours(simTimeHours) + Hours(1)) << std::endl;

  return 0;
}