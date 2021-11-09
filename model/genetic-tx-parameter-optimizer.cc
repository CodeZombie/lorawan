
#include "ns3/genetic-tx-parameter-optimizer.h"

namespace ns3
{
    namespace lorawan
    {
        NS_LOG_COMPONENT_DEFINE("GeneticTransmissionParameterOptimizer");
        NS_OBJECT_ENSURE_REGISTERED (GeneticTXParameterOptimizer);

        TypeId GeneticTXParameterOptimizer::GetTypeId(void)
        {
            static TypeId tid = TypeId("ns3::GeneticTXParameterOptimizer")
                                    .SetParent<Object>()
                                    .SetGroupName("lorawan")
                            .AddAttribute("FolderPrefix", "The folder that will be created .",
                                          StringValue("default"),
                                          MakeStringAccessor(&GeneticTXParameterOptimizer::FolderPrefix),
                                          MakeStringChecker());
            return tid;
        }
        GeneticTXParameterOptimizer::GeneticTXParameterOptimizer()
        {
            NS_LOG_INFO("Instantiating a Genetic Transmission Parameter Optimizer.");
            this->populationSize = populationSize;
            this->maxGenerations = maxGenerations;
            this->mutationRate = mutationRate;
            this->crossoverRate = crossoverRate;
            this->elitismRate = elitismRate;
            
            randomGenerator = CreateObject<UniformRandomVariable>();
            /*for(int i = 0; i < GENETIC_OPTIMIZER_POPULATION_SIZE; i++){
                tpsPopulation[i] = new TransmissionParameterSet();
            }*/

            currentTPSIndex = 0;
            for (int i = 0; i < GENETIC_OPTIMIZER_POPULATION_SIZE; i++)
            {
                currentPopulationIndices.push_back(i);
            }
            /*
            transmissionParameterSets.push_back(new TransmissionParameterSet(7, 2, 500000, 1));
            transmissionParameterSets.push_back(new TransmissionParameterSet(7, 14, 250000, 1));
            transmissionParameterSets.push_back(new TransmissionParameterSet(8, 4, 125000, 1));
            transmissionParameterSets.push_back(new TransmissionParameterSet(8, 12, 250000, 2));
            transmissionParameterSets.push_back(new TransmissionParameterSet(9, 2, 125000, 1));
            transmissionParameterSets.push_back(new TransmissionParameterSet(9, 10, 250000, 3));
            transmissionParameterSets.push_back(new TransmissionParameterSet(10, 6, 125000, 2));
            transmissionParameterSets.push_back(new TransmissionParameterSet(10, 14, 250000, 1));
            transmissionParameterSets.push_back(new TransmissionParameterSet(11, 8, 125000, 2));
            transmissionParameterSets.push_back(new TransmissionParameterSet(11, 2, 250000, 1));
            transmissionParameterSets.push_back(new TransmissionParameterSet(12, 14, 125000, 2));
            transmissionParameterSets.push_back(new TransmissionParameterSet(12, 2, 250000, 1));
            transmissionParameterSets.push_back(new TransmissionParameterSet(7, 12, 125000, 4));
            transmissionParameterSets.push_back(new TransmissionParameterSet(8, 4, 250000, 3));
            transmissionParameterSets.push_back(new TransmissionParameterSet(12, 12, 125000, 4));
            transmissionParameterSets.push_back(new TransmissionParameterSet(8, 8, 250000, 3));*/
            transmissionParameterSets.push_back(CreateObject<TransmissionParameterSet>(12, 14, 125000, 2));
            transmissionParameterSets.push_back(CreateObject<TransmissionParameterSet>(12, 14, 250000, 2));
            transmissionParameterSets.push_back(CreateObject<TransmissionParameterSet>(7, 8, 125000, 3));
            transmissionParameterSets.push_back(CreateObject<TransmissionParameterSet>(7, 2, 125000, 1));
            transmissionParameterSets.push_back(CreateObject<TransmissionParameterSet>(8, 10, 250000, 2));
            transmissionParameterSets.push_back(CreateObject<TransmissionParameterSet>(9, 6, 125000, 3));
            transmissionParameterSets.push_back(CreateObject<TransmissionParameterSet>(10, 4, 250000, 4));
            transmissionParameterSets.push_back(CreateObject<TransmissionParameterSet>(11, 10, 125000, 1));
            
            

            //shuffle the transmissionParameterSets vector
            //std::srand(0); //ensure
            //std::random_shuffle(transmissionParameterSets.begin(), transmissionParameterSets.end());

            //create a file using ofstream:
        }

        void GeneticTXParameterOptimizer::CreateLogFile(double x, double y)
        {
            std::string path = FolderPrefix + std::to_string(x) + "-" + std::to_string(y) + ".txt";
            std::cout << "CREATING LOG FILE: " << std::to_string(x) << ", " << std::to_string(y) << " Out To: " << path << std::endl;
            logFile.open(FolderPrefix + std::to_string(x) + "-" + std::to_string(y) + ".txt");
            logFile << "x,y,populationSize,maxGenerations,mutationRate,crossoverRate,elitismRate" << std::endl;
            logFile << std::to_string(x) << "," << std::to_string(y) << "," << "," << "," << std::to_string(populationSize) << "," << std::to_string(maxGenerations) << "," << std::to_string(mutationRate) << "," << std::to_string(crossoverRate) << "," << std::to_string(elitismRate) << std::endl;
        }

        void GeneticTXParameterOptimizer::PrintPopulation() {
            logFile << std::endl << "Population " << +currentGeneration << ": " << std::endl;
            for (int i = 0; i < GENETIC_OPTIMIZER_POPULATION_SIZE; i++)
            {
                
                logFile << "TPS[" << +i << "]: " << transmissionParameterSets[currentPopulationIndices[i]]->SPrint() << std::endl;
            }
        }

        void GeneticTXParameterOptimizer::PrintMasterList() {
            logFile << std::endl << "Master List:" << std::endl;
            //for every element in transmissionParameterSets

            for (uint32_t i = 0; i < transmissionParameterSets.size(); i++)
            {
                logFile << "TPS[" << +i << "]: " << transmissionParameterSets[i]->SPrint() << std::endl;
            }
        }

        Ptr<TransmissionParameterSet> GeneticTXParameterOptimizer::GetCurrentTransmissionParameterSet()
        {
            if(isOptimizing) {
                return transmissionParameterSets[currentPopulationIndices[currentTPSIndex]];
            }else{
                //get the most-fit (lowest scoring) TPS from the transmissionParameterSet list.
                return MostFitTPS;
            }
        }

        void GeneticTXParameterOptimizer::SetCurrentTransmissionParameterSetSuccess(bool successful)
        {
            if(!isOptimizing) {
                return;
            }
                GetCurrentTransmissionParameterSet()->onAckOrNack(successful);
                AdvancePopulationOrGeneration();
            
        }

        bool GeneticTXParameterOptimizer::IsOptimizing()
        {
            return isOptimizing;
        }

        void GeneticTXParameterOptimizer::AdvancePopulationOrGeneration()
        {
            if(!isOptimizing){
                return;
            }

            if (currentTPSIndex < (GENETIC_OPTIMIZER_POPULATION_SIZE - 1))
            {
                NS_LOG_INFO("Advancing to the next individual in the population");
                currentTPSIndex++;
            }
            else
            {
                PrintPopulation();
                currentGeneration++;
                if(currentGeneration == MAX_GENERATIONS) {
                    //Get the most-fit individual from the master list.
                    std::sort(transmissionParameterSets.begin(), transmissionParameterSets.end(), TransmissionParameterSet::CompareFitness);
                    MostFitTPS = transmissionParameterSets[0];
                    isOptimizing = false;
                    PrintMasterList();
                    logFile << std::endl << "MOST FIT: " << std::endl;
                    logFile << MostFitTPS->SPrint() << std::endl;
                    return;
                }

                NS_LOG_INFO("Population depleted. Generating a new Population.");
                currentTPSIndex = 0;

                //Find the most-fit individuals with a PER under TargetPER
                //if no individuals are below TargetPER, just find the lowest PER individuals.
                std::vector<Ptr<TransmissionParameterSet>> fittestTPSs = std::vector<Ptr<TransmissionParameterSet>>();

                //put the individuals from the current population into a vector.
                for (int i = 0; i < GENETIC_OPTIMIZER_POPULATION_SIZE; i++)
                {
                    fittestTPSs.push_back(transmissionParameterSets[currentPopulationIndices[i]]);
                }

                //sort this vector by Fitness (PER and PowerCons.)
                std::sort(fittestTPSs.begin(), fittestTPSs.end(), TransmissionParameterSet::CompareFitness);
                
                for(int i = 0; i < 4; i++){
                    AddToPopulation(i, fittestTPSs[i]);
                }

                
                
                //fill the population index array up with new stuff.
                for (int i = 4; i < GENETIC_OPTIMIZER_POPULATION_SIZE; i++)
                {
                    int parent_id_a = randomGenerator->GetInteger(0, 3);
                    int parent_id_b = randomGenerator->GetInteger(0, 3);
                    Ptr<TransmissionParameterSet> newTPS = CreateObject<TransmissionParameterSet>(fittestTPSs[parent_id_a], fittestTPSs[parent_id_b]);
                    bool unique = AddToPopulation(i, newTPS);
                    if(!unique) {
                        //std::cout << "AAAAAA" << std::endl;
                        //delete newTPS;
                    }
                }

                /*std::cout << "    #### NEW POPULATION ####" << std::endl;
                for(int i = 0; i < GENETIC_OPTIMIZER_POPULATION_SIZE; i++) {
                    transmissionParameterSets[currentPopulationIndices[i]]->Print();
                }*/
                //std::cout << "FInished gen new pop" << std::endl;
            }
        }

        bool GeneticTXParameterOptimizer::AddToPopulation(int offset, Ptr<TransmissionParameterSet> tps)
        {
            //check to see if this tps is identical to any other's in the major list.
            for (uint32_t i = 0; i < transmissionParameterSets.size(); i++)
            {
                if (tps->isEqual(transmissionParameterSets[i]))
                {

                    //delete tps;
                    //add i to the pop index array
                    currentPopulationIndices[offset] = i;
                    return false;
                }
            }

            //add tps pointer to the major list and then add it's index to the pop index array.
            transmissionParameterSets.push_back(tps);
            currentPopulationIndices[offset] = transmissionParameterSets.size() - 1;
            return true;
        }
    }
}