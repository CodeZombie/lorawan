#ifndef GENETIC_TX_PARAMETER_OPTIMIZER_H
#define GENETIC_TX_PARAMETER_OPTIMIZER_H

#include <algorithm>
#include <fstream>
#include "ns3/transmission-parameter-set.h"
#include "ns3/string.h"
#include "ns3/log.h"
// ! TODO: this value does not work yet. The AdvanceGeneration... method still relies on hardcoded 16 (4x4). Fix this.
//          the constructor also has a 16 hardcode.
#define MIN_SF 7
#define MAX_SF 12
#define MIN_TP 2
#define MAX_TP 16
#define MIN_BW 125000
#define MAX_BW 250000
#define MIN_CR 1
#define MAX_CR 4

namespace ns3
{
    namespace lorawan
    {
        
        class GeneticTXParameterOptimizer : public Object
        {
        public:
            static TypeId GetTypeId(void);
            GeneticTXParameterOptimizer();
            Ptr<TransmissionParameterSet> GetCurrentTransmissionParameterSet();
            void Initialize();
            void SetCurrentTransmissionParameterSetSuccess(bool successful);
            void StopOptimizing();
            bool IsOptimizing();
            void PrintPopulation();
            void PrintMasterList();

            double populationSize;
            double maxGenerations = 8;
            double mutationRate;
            double crossoverRate;
            int eliteCount;
            void CreateLogFile(double x, double y);
            std::ofstream logFile;
            std::string FolderPrefix;
            bool logFileCreated = false;
        private:
            void AdvancePopulationOrGeneration();
            bool AddToPopulation(int offset, Ptr<TransmissionParameterSet> tps);
            //A vector containing all TPSs that have been tried.
            std::vector<Ptr<TransmissionParameterSet>> transmissionParameterSets;
            
            //an array containing the indices within the above vector indicating which
            //TPSs are being tested this generations
            std::vector<int> currentPopulationIndices;

            //The index of the current index within the above array. 
            //(will always be between 0 and GENETIC_OPTIMIZER_POPULATION_SIZE)
            int currentTPSIndex = 0;

            int currentGeneration = 0;
            bool isOptimizing = true;
            Ptr<TransmissionParameterSet> MostFitTPS;
            Ptr<UniformRandomVariable> randomGenerator;


        };
    }
}
#endif