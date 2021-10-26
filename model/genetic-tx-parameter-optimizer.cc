
#include "ns3/genetic-tx-parameter-optimizer.h"

namespace ns3
{
    namespace lorawan
    {
        NS_LOG_COMPONENT_DEFINE("GeneticTransmissionParameterOptimizer");

        GeneticTXParameterOptimizer::GeneticTXParameterOptimizer()
        {
            NS_LOG_INFO("Instantiating a Genetic Transmission Parameter Optimizer.");
            randomGenerator = CreateObject<UniformRandomVariable>();
            /*for(int i = 0; i < GENETIC_OPTIMIZER_POPULATION_SIZE; i++){
                tpsPopulation[i] = new TransmissionParameterSet();
            }*/

            currentTPSIndex = 0;
            for (int i = 0; i < GENETIC_OPTIMIZER_POPULATION_SIZE; i++)
            {
                currentPopulationIndices[i] = i;
            }
            transmissionParameterSets.push_back(new TransmissionParameterSet(7, 2, 125000, 1));
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
            transmissionParameterSets.push_back(new TransmissionParameterSet(9, 12, 125000, 4));
            transmissionParameterSets.push_back(new TransmissionParameterSet(8, 8, 250000, 3));
        }

        TransmissionParameterSet *GeneticTXParameterOptimizer::GetCurrentTransmissionParameterSet()
        {
            return transmissionParameterSets[currentPopulationIndices[currentTPSIndex]];
        }

        void GeneticTXParameterOptimizer::SetCurrentTransmissionParameterSetSuccess(bool successful)
        {
            GetCurrentTransmissionParameterSet()->onAckOrNack(successful);
            AdvancePopulationOrGeneration();
        }

        void GeneticTXParameterOptimizer::AdvancePopulationOrGeneration()
        {

            if (currentTPSIndex < (GENETIC_OPTIMIZER_POPULATION_SIZE - 1))
            {
                NS_LOG_INFO("Advancing to the next individual in the population");
                currentTPSIndex++;
            }
            else
            {
                NS_LOG_INFO("Population depleted. Generating a new Population.");
                currentTPSIndex = 0;

                //Find the most-fit individuals with a PER under TargetPER
                //if no individuals are below TargetPER, just find the lowest PER individuals.
                std::vector<TransmissionParameterSet *> fittestTPSs = std::vector<TransmissionParameterSet *>();

                //put the individuals from the current population into a vector.
                for (int i = 0; i < GENETIC_OPTIMIZER_POPULATION_SIZE; i++)
                {
                    fittestTPSs.push_back(transmissionParameterSets[currentPopulationIndices[i]]);
                }

                //sort this vector by Fitness (PER and PowerCons.)
                std::sort(fittestTPSs.begin(), fittestTPSs.end(), TransmissionParameterSet::CompareFitness);

                //fill the population index array up with new stuff.
                for (int i = 0; i < GENETIC_OPTIMIZER_POPULATION_SIZE; i++)
                {
                    int parent_id_a = randomGenerator->GetInteger(0, 3);
                    int parent_id_b = randomGenerator->GetInteger(0, 3);
            
                    AddToPopulation(i, new TransmissionParameterSet(fittestTPSs[parent_id_a], fittestTPSs[parent_id_b]));
                }

                /*std::cout << "    #### NEW POPULATION ####" << std::endl;
                for(int i = 0; i < GENETIC_OPTIMIZER_POPULATION_SIZE; i++) {
                    transmissionParameterSets[currentPopulationIndices[i]]->Print();
                }*/
            }
        }

        void GeneticTXParameterOptimizer::AddToPopulation(int offset, TransmissionParameterSet *tps)
        {
            //check to see if this tps is identical to any other's in the major list.
            for (uint32_t i = 0; i < transmissionParameterSets.size(); i++)
            {
                if (tps->isEqual(transmissionParameterSets[i]))
                {
                    delete tps;
                    //add i to the pop index array
                    currentPopulationIndices[offset] = i;
                    return;
                }
            }

            //add tps pointer to the major list and then add it's index to the pop index array.
            transmissionParameterSets.push_back(tps);
            currentPopulationIndices[offset] = transmissionParameterSets.size() - 1;
        }
    }
}