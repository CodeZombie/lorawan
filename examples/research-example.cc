/*
 * This script simulates a simple network in which one end device sends one
 * packet to the gateway.
 * 
 * TODO:
 * Implement buildings.
 * Pass values from CMD to ns3::TransmissionParameterSet through Config::SetDefault.
 * Figure out a more meaningful way to convey data visually through GnuPlot. A stacking line graph is only useful in SOME situations.
 * 
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
#include "ns3/basic-energy-source.h"
#include "ns3/basic-energy-source-helper.h"
#include "ns3/lora-radio-energy-model-helper.h"
#include "ns3/file-helper.h"
#include "ns3/names.h"

#include "ns3/value-watcher.h"
#include "ns3/trace-print-helper.h"

#include "ns3/command-line.h"
#include <algorithm>
#include <ctime>

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("SimpleLorawanNetworkExample");

void printTimeLoop()
{
  std::cout << "Current Time: " << Simulator::Now().GetHours() << "h" << std::endl;
  Simulator::Schedule (Hours(100), &printTimeLoop);
}


/*******************************
             CONFIG            *
 ******************************/
//TODO: Add some Genetic Algorithm parameter customization options here. (Population size, mutation rate, etc)

bool UseGeneticAlgorithm = false;          //whether the MAC should use Genetic Algorithms or ADR.
int simTimeHours = 350;                    //How long the simulation should run for.
int NumberOfNodes = 1182;                  //The number of end-node devices in the network.
Time transmitInterval = Hours(0);          //How frequently end-nodes transmit. 0 = Random.
Time dataCaptureInterval = Hours(2);       //The time in between data sampling.
std::string adrType = "ns3::AdrComponent"; //????????
std::string outputFolder = "dat_output";   //Where output files (.dat) will be stored.
double maxRandomLoss = 0;                  //The maximum amount of random loss that can be incurred by a transmission.
double mutationRate = 0.85;                //The rate at which the Genetic Algorithm will mutate a given individual.
double crossoverRate = 0.5;                //The rate at which the Genetic Algorithm will crossover two individuals.
double elitismRate = 0.1;                  //The rate at which the Genetic Algorithm will keep the best individuals.
uint32_t populationSize = 8;              //The size of the population of individuals.
uint32_t maxGenerations = 8;              //The maximum number of generations the Genetic Algorithm will run for.

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
  cmd.AddValue("populationsize", "The size of the population of individuals per generation", populationSize);
  cmd.AddValue("maxgenerations", "The maximum number of generations the Genetic Algorithm will run for", maxGenerations);
  cmd.AddValue("mutationrate", "The maximum number of generations the Genetic Algorithm will run for", mutationRate);
  cmd.AddValue("outputfolder", "The name of the folder to stick output data into.", outputFolder);

  //cmd.AddValue("macpopulationsize", "The size of each population in the MAC Layer's Genetic Algorithm", macPopulationSize);
  //cmd.AddValue("macmutationrate", "The mutation rate of individuals in the MAC Layer's Genetic Algorithm", macMutationRate);
  //cmd.AddValue("gateways", "The number of gateways in the network", gatewayCount);
  cmd.Parse(argc, argv);

  //Setup global defaults
  Config::SetDefault ("ns3::TransmissionParameterSet::MutationRate", DoubleValue (mutationRate));
  Config::SetDefault ("ns3::GeneticTXParameterOptimizer::FolderPrefix", StringValue (outputFolder + "/GAO_Logs/"));
  Config::SetDefault ("ns3::GeneticTXParameterOptimizer::PopulationSize", UintegerValue (populationSize));
  Config::SetDefault ("ns3::GeneticTXParameterOptimizer::MaxGenerations", UintegerValue (maxGenerations));

  /*****************************
   ******** LOGGING ************
   ****************************/
  //LogComponentEnable ("AdrComponent", LOG_LEVEL_ALL);
  //LogComponentEnable("GeneticTransmissionParameterOptimizer", LOG_LEVEL_ALL);

  LogComponentEnable("EndDeviceLorawanMac", LOG_LEVEL_WARN);
  LogComponentEnable("GatewayLorawanMac", LOG_LEVEL_WARN);
  LogComponentEnable("GatewayStatus", LOG_LEVEL_WARN);
  

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
  helper.EnablePacketTracking(outputFolder + "/"); // Output filename

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
  //macHelper.Set("MType", EnumValue(LorawanMacHeader::CONFIRMED_DATA_UP));

  if (!UseGeneticAlgorithm)
  {
    macHelper.Set("EnableEDDataRateAdaptation", BooleanValue(true));
  }

  //Enable/Disable the genetic algorithm within the MAC layer.
  macHelper.Set("UseGeneticAlgorithm", BooleanValue(UseGeneticAlgorithm));

  NetDeviceContainer endDevicesNetDevices = helper.Install(phyHelper, macHelper, endDevices);



  /*******************
   * Install Energy Model
   * *******************/

  BasicEnergySourceHelper basicSourceHelper;
  LoraRadioEnergyModelHelper radioEnergyHelper;
  // configure energy source
  basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (10000)); // Energy in J
  basicSourceHelper.Set ("BasicEnergySupplyVoltageV", DoubleValue (3.3));

  radioEnergyHelper.Set ("StandbyCurrentA", DoubleValue (0.0014));
  radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.028));
  radioEnergyHelper.Set ("SleepCurrentA", DoubleValue (0.0000015));
  radioEnergyHelper.Set ("RxCurrentA", DoubleValue (0.0112));

  radioEnergyHelper.SetTxCurrentModel ("ns3::ConstantLoraTxCurrentModel", "TxCurrent", DoubleValue (0.028));

  // install source on EDs' nodes
  EnergySourceContainer energySources = basicSourceHelper.Install (endDevices);

  // install device model
  DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install(endDevicesNetDevices, energySources);

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
  macHelper.SetRegion (LorawanMacHelper::EU);
  helper.Install(phyHelper, macHelper, gateways);

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
    (*j)->GetDevice(0)->GetObject<LoraNetDevice>()->GetMac()->SetAttribute("Location_X", DoubleValue(position.x));
    (*j)->GetDevice(0)->GetObject<LoraNetDevice>()->GetMac()->SetAttribute("Location_Y", DoubleValue(position.y));
  }

  /**************************************
   *  Setup Tracing and Data Collection  *
   **************************************/
  
  TracePrintHelper *tracePrintHelper;
  tracePrintHelper = new TracePrintHelper(dataCaptureInterval);
  //Setup watchers.
  tracePrintHelper->AddValueWatcher(new ValueWatcher("TotalEnergyConsumption", &deviceModels, ValueWatcher::Type::Double, ValueWatcher::CombineMode::Sum, ValueWatcher::SourceType::Attribute, outputFolder + "/"));
  tracePrintHelper->AddValueWatcher(new ValueWatcher("PacketErrorRate", "/NodeList/*/DeviceList/*/$ns3::LoraNetDevice/Mac/$ns3::ClassAEndDeviceLorawanMac", ValueWatcher::Type::Double, ValueWatcher::CombineMode::None, ValueWatcher::SourceType::Attribute, outputFolder + "/"));
  tracePrintHelper->AddValueWatcher(new ValueWatcher("TransmissionsSent", "/NodeList/*/DeviceList/*/$ns3::LoraNetDevice/Mac/$ns3::ClassAEndDeviceLorawanMac", ValueWatcher::Type::Integer, ValueWatcher::CombineMode::Sum, ValueWatcher::SourceType::Attribute, outputFolder + "/"));
  tracePrintHelper->AddValueWatcher(new ValueWatcher("LastNPSR", "/NodeList/*/DeviceList/*/$ns3::LoraNetDevice/Mac/$ns3::ClassAEndDeviceLorawanMac", ValueWatcher::Type::Double, ValueWatcher::CombineMode::None, ValueWatcher::SourceType::Attribute, outputFolder + "/"));
  tracePrintHelper->AddValueWatcher(new ValueWatcher("DataRate", "/NodeList/*/DeviceList/*/$ns3::LoraNetDevice/Mac/$ns3::ClassAEndDeviceLorawanMac", ValueWatcher::Type::Uinteger, ValueWatcher::CombineMode::None, ValueWatcher::SourceType::Attribute, outputFolder + "/"));
  tracePrintHelper->AddValueWatcher(new ValueWatcher("LastFitnessLevel", "/NodeList/*/DeviceList/*/$ns3::LoraNetDevice/Mac/$ns3::ClassAEndDeviceLorawanMac", ValueWatcher::Type::Double, ValueWatcher::CombineMode::None, ValueWatcher::SourceType::Attribute, outputFolder + "/"));
  tracePrintHelper->AddValueWatcher(new ValueWatcher("FailedTransmissionCount", "/NodeList/*/DeviceList/*/$ns3::LoraNetDevice/Mac/$ns3::ClassAEndDeviceLorawanMac", ValueWatcher::Type::Integer, ValueWatcher::CombineMode::None, ValueWatcher::SourceType::Attribute, outputFolder + "/"));
  tracePrintHelper->AddValueWatcher(new ValueWatcher("MType", "/NodeList/*/DeviceList/*/$ns3::LoraNetDevice/Mac/$ns3::ClassAEndDeviceLorawanMac", ValueWatcher::Type::Enum, ValueWatcher::CombineMode::None, ValueWatcher::SourceType::Attribute, outputFolder + "/"));
  tracePrintHelper->AddValueWatcher(new ValueWatcher("PacketsSent", "/NodeList/*/ApplicationList/*/$ns3::PeriodicSender", ValueWatcher::Type::Integer, ValueWatcher::CombineMode::Sum, ValueWatcher::SourceType::TraceSource,  outputFolder + "/"));


  tracePrintHelper->Start();

  /****************
  *  Simulation  *
  ****************/
  Simulator::Schedule (Hours(100), &printTimeLoop);

  Simulator::Stop(Hours(simTimeHours));
  Simulator::Run();
  Simulator::Destroy();
  LoraPacketTracker &tracker = helper.GetPacketTracker();
  tracker.PrintDiscretePSR(outputFolder + "/", Hours(simTimeHours / 16 ));
  std::cout << tracker.CountMacPacketsGlobally(Seconds(0), Hours(simTimeHours) + Hours(1)) << std::endl;
  std::cout << tracker.CountMacPacketsGloballyCpsr(Seconds(0), Hours(simTimeHours) + Hours(1)) << std::endl;

  return 0;
}