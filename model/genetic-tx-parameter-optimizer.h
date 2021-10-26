#ifndef GENETIC_TX_PARAMETER_OPTIMIZER_H
#define GENETIC_TX_PARAMETER_OPTIMIZER_H

#include <algorithm>
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
            TransmissionParameterSet *GetCurrentTransmissionParameterSet();
            void SetCurrentTransmissionParameterSetSuccess(bool successful);

        private:
            void AdvancePopulationOrGeneration();
            void AddToPopulation(int offset, TransmissionParameterSet* tps);
            //A vector containing all TPSs that have been tried.
            std::vector<TransmissionParameterSet*> transmissionParameterSets;
            
            //an array containing the indices within the above vector indicating which
            //TPSs are being tested this generations
            int currentPopulationIndices[GENETIC_OPTIMIZER_POPULATION_SIZE];

            //The index of the current index within the above array. 
            //(will always be between 0 and GENETIC_OPTIMIZER_POPULATION_SIZE)
            int currentTPSIndex = 0;

            
            
            Ptr<UniformRandomVariable> randomGenerator;
        };
    }
}
#endif