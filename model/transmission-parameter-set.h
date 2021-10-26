#ifndef TRANSMISSION_PARAMETER_SET_H
#define TRANSMISSION_PARAMETER_SET_H
#include "ns3/log.h"
#include "ptr.h"
#include "random-variable-stream.h"
#include <math.h>
namespace ns3
{
    namespace lorawan
    {

        class TransmissionParameterSet
        {
        public:
            TransmissionParameterSet();
            TransmissionParameterSet(TransmissionParameterSet* other);
            TransmissionParameterSet(int sf, int pow, int bw, int cr);
            TransmissionParameterSet(TransmissionParameterSet *parent_a, TransmissionParameterSet *parent_b);

            float getPER();
            int mutateValue(int originalValue, int delta, int min, int max);
            void Print();
            float fitness();
            bool isEqual(TransmissionParameterSet *other);
            int dataRate();
            void onAckOrNack(bool successful);
            float PowerConsumption();

            static float PowerConsumption(uint8_t spreadingfactor, uint32_t bandwidth, int codingrate, float power);

            int power;
            int spreadingFactor;
            int bandwidth;
            int codingRate;

            int successCount = 0;
            int failureCount = 0;

            static bool CompareFitness(TransmissionParameterSet *a, TransmissionParameterSet *b);

        private:
            Ptr<UniformRandomVariable> randomGenerator;
            void initializeRNG();
        };

    }
}
#endif