#ifndef TRANSMISSION_PARAMETER_SET_H
#define TRANSMISSION_PARAMETER_SET_H
#include "ns3/object.h"
#include "ns3/log.h"
#include "ptr.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"
#include "random-variable-stream.h"
#include <math.h>
namespace ns3
{
    namespace lorawan
    {

        class TransmissionParameterSet : public Object
        {
        public:
            static TypeId GetTypeId(void);
            TransmissionParameterSet();
            TransmissionParameterSet(Ptr<TransmissionParameterSet> other);
            TransmissionParameterSet(int sf, int pow, int bw, int cr);
            TransmissionParameterSet(Ptr<TransmissionParameterSet> parent_a, Ptr<TransmissionParameterSet> parent_b);
            TransmissionParameterSet(Ptr<TransmissionParameterSet> parent_a, Ptr<TransmissionParameterSet> parent_b, int pivot);

            void mutate();
            float getPER();
            int mutateValue(int originalValue, int delta, int min, int max);
            void Print();
            std::string SPrint();
            float fitness();
            bool isEqual(Ptr<TransmissionParameterSet> other);
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

            static bool CompareFitness(Ptr<TransmissionParameterSet> a, Ptr<TransmissionParameterSet> b);

            double mutationRate;

        private:
            Ptr<UniformRandomVariable> randomGenerator;
            void initializeRNG();
        };

    }
}
#endif