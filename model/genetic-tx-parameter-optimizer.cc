
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
            currentTPS = new TransmissionParameterSet(currentSF, currentTP, currentBW, currentCR);
            /*for(int i = 0; i < GENETIC_OPTIMIZER_POPULATION_SIZE; i++){
                tpsPopulation[i] = new TransmissionParameterSet();
            }*/
            /*currentPopulationIndices = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
            transmissionParameterSet.push_back(new TransmissionParameterSet(7, 2, 125000, 1));
            transmissionParameterSet.push_back(new TransmissionParameterSet(7, 14, 250000, 1));
            transmissionParameterSet.push_back(new TransmissionParameterSet(8, 4, 125000, 1));
            transmissionParameterSet.push_back(new TransmissionParameterSet(8, 12, 250000, 2));
            transmissionParameterSet.push_back(new TransmissionParameterSet(9, 2, 125000, 1));
            transmissionParameterSet.push_back(new TransmissionParameterSet(9, 10, 250000, 3));
            transmissionParameterSet.push_back(new TransmissionParameterSet(10, 6, 125000, 2));
            transmissionParameterSet.push_back(new TransmissionParameterSet(10, 14, 250000, 1));
            transmissionParameterSet.push_back(new TransmissionParameterSet(11, 8, 125000, 2));
            transmissionParameterSet.push_back(new TransmissionParameterSet(11, 2, 250000, 1));
            transmissionParameterSet.push_back(new TransmissionParameterSet(12, 14, 125000, 2));
            transmissionParameterSet.push_back(new TransmissionParameterSet(12, 2, 250000, 1));
            transmissionParameterSet.push_back(new TransmissionParameterSet(7, 12, 125000, 4));
            transmissionParameterSet.push_back(new TransmissionParameterSet(8, 4, 250000, 3));
            transmissionParameterSet.push_back(new TransmissionParameterSet(9, 12, 125000, 4));
            transmissionParameterSet.push_back(new TransmissionParameterSet(8, 8, 250000, 3));*/
        }

        int GeneticTXParameterOptimizer::GetSuccesfulParameterSets()
        {
            /*
            int successfulSets = 0;
            for (int i = 0; i < GENETIC_OPTIMIZER_POPULATION_SIZE; i++)
            {
                if (tpsPopulation[i]->successful)
                {
                    successfulSets++;
                }
            }
            return successfulSets;*/
            return -1;
        }



        TransmissionParameterSet *GeneticTXParameterOptimizer::GetCurrentTransmissionParameterSet()
        {
            return currentTPS;
            //return currentPopulationIndices[currentTPSIndex];
        }

        void GeneticTXParameterOptimizer::SetCurrentTransmissionParameterSetSuccess(bool successful)
        {
            //add this tps to the list of successful tps' if successful
            /*
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
            */

            /*if (successful)
            {
                transmissionParameterSets[currentPopulationIndices[currentTPSIndex]]->successCount++;
            }
            else
            {
                transmissionParameterSets[currentPopulationIndices[currentTPSIndex]]->failureCount++;
            }*/
            //currentTPS->Print();
            std::cout << "Successful = " << successful << std::endl;
            if(successful == 0){
                std::cout << "#############################################" << std::endl;
                std::cout << "################ FAILED #####################" << std::endl;
                std::cout << "#############################################" << std::endl;
            }
            AdvancePopulationOrGeneration();
        }

        void GeneticTXParameterOptimizer::AdvancePopulationOrGeneration()
        {
            //this is vile.
            if(currentSF < MAX_SF){
                currentSF++;
            }else{
                currentSF = MIN_SF;
                if(currentTP < MAX_TP){
                    currentTP++;
                }else{
                    currentTP = MIN_TP;
                    if(currentBW < MAX_BW){
                        currentBW *= 2;
                    }else{
                        currentBW = MIN_BW;
                        if(currentCR < MAX_CR){
                            currentCR++;
                        }else{
                            currentCR = MIN_CR;
                        }
                    }
                }
            }
            //std::cout << "SF: " << currentSF << "  TP: " << currentTP << "  BW: " << currentBW << "  CR: " << currentCR << std::endl;
            
            delete currentTPS;
            currentTPS = new TransmissionParameterSet(currentSF, currentTP, currentBW, currentCR);
            return;
            /*
            //NS_LOG_INFO("Advancing to the next individual in the population");
            if (currentTPSIndex < (GENETIC_OPTIMIZER_POPULATION_SIZE - 1))
            {
                currentTPSIndex++;
            }
            else
            {

                currentTPSIndex = 0;

                //Find the most-fit individuals with a PER under 0.1
                //if no individuals are below 0.1, just find the lowest PER individuals.
                for (auto iter = transmissionParameterSets.begin(); iter != transmissionParameterSets.end(); iter++)
                {
                    //find all individuals with a PER sub 0.1.
                    //These individuals will be sorted by fitness in the next block.
                }

                //sort by fitness.

                //choose the top 4 and mutate/cross them into 16 new individuals.

                //its likely that some of these new individuals wont be "new", so make sure to check
                //their originality within transmissionParameterSets vector before adding.

                //if there arent 4 individuals below 0.1 PER, choose the lowest PERs, ignoring fitness.

                

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
                
                for (int x = 0; x < 4; x++)
                {
                    for (int y = 0; y < 4; y++)
                    {
                        tpsPopulation[(y * 4) + x] = new TransmissionParameterSet(fitpop[x], fitpop[y]);
                    }
                }*/

                /*

                //pass the 4 parents to the new generation.
                for (int i = 0; i < 4; i++)
                {
                    tpsPopulation[i] = fitpop[i];
                    tpsPopulation[i]->successful = -1;
                }

                //create n - 4 new individuals. They must ALL be unique.
                for (int i = 4; i < GENETIC_OPTIMIZER_POPULATION_SIZE; i++)
                {
                    
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
                   /*
                    int parent_id_a = randomGenerator->GetInteger(0, 3);
                    int parent_id_b = randomGenerator->GetInteger(0, 3);
                    tpsPopulation[i] = new TransmissionParameterSet(fitpop[parent_id_a], fitpop[parent_id_b]);
                }
            }
            */
        }

        int GeneticTXParameterOptimizer::GetIndexOfFittestTPSInPopulation()
        {
            /*
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
            */
           return -1;
        }
    }
}