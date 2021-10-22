
#include "ns3/genetic-tx-parameter-optimizer.h"

namespace ns3
{
    namespace lorawan
    {
        NS_LOG_COMPONENT_DEFINE("GeneticTransmissionParameterOptimizer");

        GeneticTXParameterOptimizer::GeneticTXParameterOptimizer()
        {
            NS_LOG_INFO("Instantiating a Genetic Transmission Parameter Optimizer.");
            currentTPSIndex = 0;
            randomGenerator = CreateObject<UniformRandomVariable>();
            successfulTransmissionParameterSets = std::vector<TransmissionParameterSet *>();
            /*for(int i = 0; i < GENETIC_OPTIMIZER_POPULATION_SIZE; i++){
                tpsPopulation[i] = new TransmissionParameterSet();
            }*/

            tpsPopulation[0] = new TransmissionParameterSet(7, 2, 125000, 1);
            tpsPopulation[1] = new TransmissionParameterSet(7, 14, 250000, 1);
            tpsPopulation[2] = new TransmissionParameterSet(8, 4, 125000, 1);
            tpsPopulation[3] = new TransmissionParameterSet(8, 12, 250000, 2);
            tpsPopulation[4] = new TransmissionParameterSet(9, 2, 125000, 1);
            tpsPopulation[5] = new TransmissionParameterSet(9, 10, 250000, 3);
            tpsPopulation[6] = new TransmissionParameterSet(10, 6, 125000, 2);
            tpsPopulation[7] = new TransmissionParameterSet(10, 14, 250000, 1);
            tpsPopulation[8] = new TransmissionParameterSet(11, 8, 125000, 2);
            tpsPopulation[9] = new TransmissionParameterSet(11, 2, 250000, 1);
            tpsPopulation[10] = new TransmissionParameterSet(12, 14, 125000, 2);
            tpsPopulation[11] = new TransmissionParameterSet(12, 2, 250000, 1);
            tpsPopulation[12] = new TransmissionParameterSet(7, 12, 125000, 4);
            tpsPopulation[13] = new TransmissionParameterSet(8, 4, 250000, 3);
            tpsPopulation[14] = new TransmissionParameterSet(9, 12, 125000, 4);
            tpsPopulation[15] = new TransmissionParameterSet(8, 8, 250000, 3);
        }

        int GeneticTXParameterOptimizer::GetSuccesfulParameterSets()
        {
            int successfulSets = 0;
            for (int i = 0; i < GENETIC_OPTIMIZER_POPULATION_SIZE; i++)
            {
                if (tpsPopulation[i]->successful)
                {
                    successfulSets++;
                }
            }
            return successfulSets;
        }

        TransmissionParameterSet *GeneticTXParameterOptimizer::GetCurrentTransmissionParameterSet()
        {
            return tpsPopulation[currentTPSIndex];
        }

        void GeneticTXParameterOptimizer::SetCurrentTransmissionParameterSetSuccess(bool successful)
        {
            //add this tps to the list of successful tps' if successful
            if (successful)
            {
                bool isInVectorAlready = false;
                for (auto iter = successfulTransmissionParameterSets.begin(); iter != successfulTransmissionParameterSets.end(); iter++)
                {
                    if (tpsPopulation[currentTPSIndex]->isEqual((*iter)))
                    {
                        (*iter)->successCount++;
                        isInVectorAlready = true;
                        break;
                    }
                }
                if (isInVectorAlready == false)
                {
                    //std::cout << "Adding TPS to Success List: " << std::endl;
                    //tpsPopulation[currentTPSIndex]->Print();
                    TransmissionParameterSet *tps = new TransmissionParameterSet(tpsPopulation[currentTPSIndex]);
                    tps->successCount++;
                    successfulTransmissionParameterSets.push_back(tps);
                }
            }
            tpsPopulation[currentTPSIndex]->successful = successful;
            AdvancePopulationOrGeneration();
        }

        void GeneticTXParameterOptimizer::AdvancePopulationOrGeneration()
        {

            //NS_LOG_INFO("Advancing to the next individual in the population");
            if (currentTPSIndex < (GENETIC_OPTIMIZER_POPULATION_SIZE - 1))
            {
                currentTPSIndex++;
            }
            else
            {
                /* TODO: This algorithm is kinda whack?? The for loop with the "delete tpsPopulation[i]" is not making sense. */

                NS_LOG_INFO("Entire population has been tested. Succesful: " << GetSuccesfulParameterSets());

                //TEST:
                //If zero new parameters are succesful, we just reset the same params and try again
                /*if(GetSuccesfulParameterSets() == 0){
                    for(int i = 0; i < GENETIC_OPTIMIZER_POPULATION_SIZE; i++){
                        tpsPopulation[i]->successful = -1;
                    }
                    currentTPSIndex = 0;
                    return;
                }*/

                //Get all succesful individuals and sort them from most fit to least.
                //Take the top 4, generating new random ones if there are not 4 (NOTE: weighted toward being longer range)

                currentTPSIndex = 0;
                TransmissionParameterSet *fitpop[4];
                for (int i = 0; i < 4; i++)
                {
                    int fittestIndex = GetIndexOfFittestTPSInPopulation();
                    if (fittestIndex == -1)
                    {
                        //if no fittest was found, generate a new one randomly.
                        //NOTE: we want to insert some bias here toward more energy-intensive settings as if we get here it means
                        //  we probably need stronger settings.
                        //Instead of generating some random thing, randomly grab one of the previously successful params.

                        //fitpop[i] = new TransmissionParameterSet(12, 14, 125000, 4);
                        //std::cout << "Pulling from stps " << std::endl;
                        
                        fitpop[i] = new TransmissionParameterSet(successfulTransmissionParameterSets[rand() % successfulTransmissionParameterSets.size()]);
                    }
                    else
                    {
                        fitpop[i] = tpsPopulation[fittestIndex];
                        tpsPopulation[fittestIndex] = NULL;
                    }
                }

                NS_LOG_INFO("Most-fit individual: " << fitpop[0]->fitness() << ".");

                //delete all remaining unfit/unsuccesful objects from population.
                //At this point every element in the tpsPopulation array will be NULL.
                for (int i = 0; i < GENETIC_OPTIMIZER_POPULATION_SIZE; i++)
                {
                    if (tpsPopulation[i] != NULL)
                    {
                        delete tpsPopulation[i];
                        tpsPopulation[i] = NULL;
                    }
                }

                //create a new population by combining the 4 fit individuals.
                /*
                for (int x = 0; x < 4; x++)
                {
                    for (int y = 0; y < 4; y++)
                    {
                        tpsPopulation[(y * 4) + x] = new TransmissionParameterSet(fitpop[x], fitpop[y]);
                    }
                }*/

                //pass the 4 parents to the new generation.
                for (int i = 0; i < 4; i++)
                {
                    tpsPopulation[i] = fitpop[i];
                    tpsPopulation[i]->successful = -1;
                }

                //create n - 4 new individuals. They must ALL be unique.
                for (int i = 4; i < GENETIC_OPTIMIZER_POPULATION_SIZE; i++)
                {
                    /*
                    bool unique = false;
                    TransmissionParameterSet *newIndividual;
                    while (!unique)
                    {
                        unique = true;
                        int parent_id_a = randomGenerator->GetInteger(0, 3);
                        int parent_id_b = randomGenerator->GetInteger(0, 3);
                        newIndividual = new TransmissionParameterSet(fitpop[parent_id_a], fitpop[parent_id_b]);

                        //set unique to false if the `newIndividual` is equal to any other individuals in the population.
                        //Then delete newIndividual because we'll need to create it again.
                        for (int j = 0; j < GENETIC_OPTIMIZER_POPULATION_SIZE; j++)
                        {
                            if (tpsPopulation[i] == NULL)
                            {
                                break;
                            }
                            if (tpsPopulation[i]->isEqual(newIndividual))
                            {
                                unique = false;
                                delete newIndividual;
                            }
                        }
                    }
                    tpsPopulation[i] = newIndividual;
                    */
                    int parent_id_a = randomGenerator->GetInteger(0, 3);
                    int parent_id_b = randomGenerator->GetInteger(0, 3);
                    tpsPopulation[i] = new TransmissionParameterSet(fitpop[parent_id_a], fitpop[parent_id_b]);
                }
            }
        }

        int GeneticTXParameterOptimizer::GetIndexOfFittestTPSInPopulation()
        {
            int fittestIndex = -1;
            for (int i = 0; i < GENETIC_OPTIMIZER_POPULATION_SIZE; i++)
            {
                if (tpsPopulation[i] == NULL)
                {
                    continue;
                }
                if (tpsPopulation[i]->successful == 0)
                {
                    continue;
                }
                if (fittestIndex == -1 || tpsPopulation[i]->fitness() > tpsPopulation[fittestIndex]->fitness())
                {
                    fittestIndex = i;
                }
            }

            return fittestIndex;
        }
    }
}