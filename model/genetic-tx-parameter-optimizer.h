#ifndef GENETIC_TX_PARAMETER_OPTIMIZER_H
#define GENETIC_TX_PARAMETER_OPTIMIZER_H

#include "ns3/transmission-parameter-set.h"
#include "ns3/log.h"

// ! TODO: this value does not work yet. The AdvanceGeneration... method still relies on hardcoded 16 (4x4). Fix this.
//          the constructor also has a 16 hardcode.
#define GENETIC_OPTIMIZER_POPULATION_SIZE 16
#define MIN_SF 7
#define MAX_SF 12
#define MIN_TP 2
#define MAX_TP 14
#define MIN_BW 125000
#define MAX_BW 500000
#define MIN_CR 1
#define MAX_CR 4

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
            //TransmissionParameterSet *tpsPopulation[GENETIC_OPTIMIZER_POPULATION_SIZE];
            int currentPopulationIndices[GENETIC_OPTIMIZER_POPULATION_SIZE];
            int currentTPSIndex = 0;

            //test
            int currentSF = 7;
            int currentTP = 2;
            int currentBW = 125000;
            int currentCR = 1;
            TransmissionParameterSet* currentTPS;

            std::vector<TransmissionParameterSet*> transmissionParameterSets;
            
            Ptr<UniformRandomVariable> randomGenerator;
        };
    }
}
#endif