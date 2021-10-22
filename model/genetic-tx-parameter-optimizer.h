#ifndef GENETIC_TX_PARAMETER_OPTIMIZER_H
#define GENETIC_TX_PARAMETER_OPTIMIZER_H

#include "ns3/transmission-parameter-set.h"
#include "ns3/log.h"

// ! TODO: this value does not work yet. The AdvanceGeneration... method still relies on hardcoded 16 (4x4). Fix this.
//          the constructor also has a 16 hardcode.
#define GENETIC_OPTIMIZER_POPULATION_SIZE 16

namespace ns3
{
    namespace lorawan
    {

        class GeneticTXParameterOptimizer
        {
        public:
            GeneticTXParameterOptimizer();
            int GetSuccesfulParameterSets();
            TransmissionParameterSet *GetCurrentTransmissionParameterSet();
            void SetCurrentTransmissionParameterSetSuccess(bool successful);

        private:
            void AdvancePopulationOrGeneration();
            int GetIndexOfFittestTPSInPopulation();
            TransmissionParameterSet *tpsPopulation[GENETIC_OPTIMIZER_POPULATION_SIZE];
            std::vector<TransmissionParameterSet*> successfulTransmissionParameterSets;
            int currentTPSIndex = 0;
            Ptr<UniformRandomVariable> randomGenerator;
        };
    }
}
#endif