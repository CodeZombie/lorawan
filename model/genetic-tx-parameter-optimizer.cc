
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

        TransmissionParameterSet *GeneticTXParameterOptimizer::GetCurrentTransmissionParameterSet()
        {
            return tpsPopulation[currentTPSIndex];
        }

        void GeneticTXParameterOptimizer::SetCurrentTransmissionParameterSetSuccess(bool successful)
        {
            tpsPopulation[currentTPSIndex]->successful = successful;
            AdvancePopulationOrGeneration();
        }

        void GeneticTXParameterOptimizer::AdvancePopulationOrGeneration()
        {
            NS_LOG_INFO("Advancing to the next individual in the population");
            if (currentTPSIndex < (GENETIC_OPTIMIZER_POPULATION_SIZE - 1))
            {
                currentTPSIndex++;
            }
            else
            {
                /* TODO: This algorithm is kinda whack?? The for loop with the "delete tpsPopulation[i]" is not making sense. */

                NS_LOG_INFO("Entire population has been tested. Generating a new population...");
                
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
                        fitpop[i] = new TransmissionParameterSet();
                    }
                    else
                    {
                        fitpop[i] = tpsPopulation[fittestIndex];
                        tpsPopulation[fittestIndex] = NULL;
                    }
                }

                //delete all remaining unfit/unsuccesful objects from population.
                for (int i = 0; i < 16; i++)
                {
                    if (tpsPopulation[i] != NULL)
                    {
                        delete tpsPopulation[i];
                    }
                }

                //create a new population by combining the 4 fit individuals.
                for (int x = 0; x < 4; x++)
                {
                    for (int y = 0; y < 4; y++)
                    {
                        tpsPopulation[(y * 4) + x] = new TransmissionParameterSet(fitpop[x], fitpop[y]);
                    }
                }

                //delete all objects from fitpop.
                for (int i = 0; i < 4; i++)
                {
                    delete fitpop[i];
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