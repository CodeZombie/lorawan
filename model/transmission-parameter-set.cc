#include "ns3/transmission-parameter-set.h"
//TODO:
//      Refine the valid ranges of data. For example, TXp 14 and BW 500k I think is invalid.
//      The system should check for that.
namespace ns3
{
    namespace lorawan
    {

        NS_LOG_COMPONENT_DEFINE("TransmissionParameterSet");
        NS_OBJECT_ENSURE_REGISTERED(TransmissionParameterSet);

        TypeId TransmissionParameterSet::GetTypeId(void)
        {
            static TypeId tid = TypeId("ns3::TransmissionParameterSet")
                                    .SetParent<Object>()
                                    .SetGroupName("lorawan")
                                    .AddAttribute("MutationRate", "The mutation rate of the transmission parameter set.",
                                                  DoubleValue(0.9),
                                                  MakeDoubleAccessor(&TransmissionParameterSet::mutationRate),
                                                  MakeDoubleChecker<double>())
                                    .AddAttribute("CrossoverRate", "The crossover rate of the transmission parameter set.",
                                                  DoubleValue(0.9),
                                                  MakeDoubleAccessor(&TransmissionParameterSet::crossoverRate),
                                                  MakeDoubleChecker<double>());
            return tid;
        }

        TransmissionParameterSet::TransmissionParameterSet()
        {
            initializeRNG();
            //spreading factor: 7 to 12
            spreadingFactor = randomGenerator->GetInteger(7, 12);
            //power: 2, 4, 6, 8, 10, 12, 14, 16
            power = randomGenerator->GetInteger(1, 8) * 2; //8
            //bandwidth: 125000, 250000, 500000
            int choice = randomGenerator->GetInteger(1, 2); /// 2????
            if (choice == 1)
            {
                bandwidth = 125000;
            }
            else
            {
                bandwidth = 250000;
            }
            //coding rate: 1 (4/5), 2 (4/6), 3 (4/7), 4 (4/8)
            codingRate = randomGenerator->GetInteger(1, 4);
        }

        bool TransmissionParameterSet::isEqual(Ptr<TransmissionParameterSet> other)
        {
            bool equality = true;
            if (spreadingFactor != other->spreadingFactor)
            {
                equality = false;
            }

            if (power != other->power)
            {
                equality = false;
            }

            if (bandwidth != other->bandwidth)
            {
                equality = false;
            }

            if (codingRate != other->codingRate)
            {
                equality = false;
            }

            return equality;
        }

        TransmissionParameterSet::TransmissionParameterSet(int sf, int pow, int bw, int cr)
        {
            initializeRNG();
            spreadingFactor = sf;
            power = pow;
            bandwidth = bw;
            codingRate = cr;
        }

        TransmissionParameterSet::TransmissionParameterSet(Ptr<TransmissionParameterSet> other)
        {
            initializeRNG();
            spreadingFactor = other->spreadingFactor;
            power = other->power;
            bandwidth = other->bandwidth;
            codingRate = other->codingRate;
        }

        void TransmissionParameterSet::Crossover(Ptr<TransmissionParameterSet> parent_a, Ptr<TransmissionParameterSet> parent_b, int pivot)
        {
            //Check to see if a crossover should even occur.
            //NOTE: This should be randm() < crossoverRate not the other way around lol
            if (randomGenerator->GetValue(0, 1) < crossoverRate)
            {
                //Do not crossover. Simply duplicate the settings from parent_a and end;
                this->spreadingFactor = parent_a->spreadingFactor;
                this->power = parent_a->power;
                this->bandwidth = parent_a->bandwidth;
                this->codingRate = parent_a->codingRate;
                return;
            }

            //generate parameters based on parent's parameters, pivot point.
            if (pivot == 1)
            {
                //pivot is SF
                spreadingFactor = parent_a->spreadingFactor;
                power = parent_b->power;
                bandwidth = parent_b->bandwidth;
                codingRate = parent_b->codingRate;
            }
            else if (pivot == 2)
            {
                //pivot is power
                spreadingFactor = parent_a->spreadingFactor;
                power = parent_a->power;
                bandwidth = parent_b->bandwidth;
                codingRate = parent_b->codingRate;
            }
            else if (pivot == 3)
            {
                //pivot is cr
                spreadingFactor = parent_a->spreadingFactor;
                power = parent_a->power;
                bandwidth = parent_a->bandwidth;
                codingRate = parent_b->codingRate;
            }
        }

        void TransmissionParameterSet::Mutate()
        {

            //mutate SF
            if (randomGenerator->GetValue(0, 1) < mutationRate)
            {
                spreadingFactor = mutateValue(spreadingFactor, 1, 7, 12);
            }

            //mutate power
            if (randomGenerator->GetValue(0, 1) < mutationRate)
            {
                power = mutateValue(power, 2, 2, 14);
            }

            //mutate bandwidth
            if (randomGenerator->GetValue(0, 1) < mutationRate)
            {
                if (bandwidth == 125000)
                {
                    bandwidth = 250000;
                }
                else if (bandwidth == 250000)
                {
                    bandwidth = 125000;
                }
            }

            //mutate cr
            if (randomGenerator->GetValue(0, 1) < mutationRate)
            {
                codingRate = mutateValue(codingRate, 1, 1, 4);
            }
        }

        float TransmissionParameterSet::getPER()
        {
            int totalTransmissions = successCount + failureCount;
            if (totalTransmissions == 0)
            {
                return 0.0;
            }
            return (float)failureCount / (float)totalTransmissions;
        }

        int TransmissionParameterSet::mutateValue(int originalValue, int delta, int min, int max)
        {
            /*
          Mutates a value by shifting it up or down within a range.
          params:
          int originalValue: The original integer value to be mutated.
          int delta: the size of the step, up or down, that the number will mutate by.
          int min: the minimum value of the range that this method can return.
          int max: the maximum value of the range this method can return.
      */
            if (originalValue == min)
            {
                return originalValue + delta;
            }
            else if (originalValue == max)
            {
                return originalValue - delta;
            }
            else
            {
                int mutate_direction = randomGenerator->GetInteger(0, 1);
                if (mutate_direction == 0)
                {
                    return originalValue + delta;
                }
                else
                {
                    return originalValue - delta;
                }
            }
        }

        void TransmissionParameterSet::initializeRNG()
        {
            randomGenerator = CreateObject<UniformRandomVariable>();
        }

        void TransmissionParameterSet::Print()
        {
            //NS_LOG_INFO("TXPARAMS: SF=" << spreadingFactor << " PW=" << power << " BW=" << bandwidth << " CR=" << codingRate << " FITNESS=" << fitness() << " SUCCESS= " << successful);
            std::cout << SPrint() << std::endl;
        }

        std::string TransmissionParameterSet::SPrint()
        {
            return "TPS: SF=" + std::to_string(spreadingFactor) +
                   " PW=" + std::to_string(power) +
                   " BW=" + std::to_string(bandwidth) +
                   " CR=" + std::to_string(codingRate) +
                   " FITNESS=" + std::to_string(fitness()) +
                   " PER=" + std::to_string(getPER()) +
                   " txSUCCESS= " + std::to_string(successCount) +
                   " txFAILURE= " + std::to_string(failureCount) +
                   " 12-byte Energy (mJ): " + std::to_string(PCON(96.0, spreadingFactor, bandwidth, codingRate, power));
        }

        void TransmissionParameterSet::onAckOrNack(bool successful)
        {
            if (successful)
            {
                successCount++;
            }
            else
            {
                failureCount++;
            }
        }

        bool TransmissionParameterSet::CompareFitness(Ptr<TransmissionParameterSet> a, Ptr<TransmissionParameterSet> b)
        {
            return a->fitness() < b->fitness();
        }

        //  Fitness function: lower is better.
        float TransmissionParameterSet::fitness()
        {
            if (getPER() == 1.0)
            {
                return 999999;
            }
            //Calculate the number of retries that are needed to succesfully transmit a packet given the current Packet Error Rate:
            float PER = getPER();
            float retries = log(1 - PER) / log(PER);
            float powerConsumption = PowerConsumption(this->spreadingFactor, this->bandwidth, this->codingRate, this->power);
            return powerConsumption + (powerConsumption * retries);

            //float powerConsumption = PowerConsumption(this->spreadingFactor, this->bandwidth, this->codingRate, this->power) * 0.5f;
            //return powerConsumption + (powerConsumption * getPER());
        }

        float TransmissionParameterSet::PowerConsumption()
        {
            return PowerConsumption(this->spreadingFactor, this->bandwidth, this->codingRate, this->power);
        }

        //OPtions:
        /*
        * Lets say we have 5 TPSs.
        1. PER: 0.1 TOTALPOW: 30
        2. PER: 0.2 TOTALPOW: 20
        3. PER: 1.0 TOTALPOW: 05
        4. PER: 0.9 TOTALPOW: 10

        we probably want it to choose either 1 or 2, but not 3 or 4.

        */

       float TransmissionParameterSet::PCON(double bits, uint8_t sf, uint32_t b, int cr, float p) {
           float cr_ = 4.0 / (cr + 1.0); //Convert Coding Rate from an integer into the ratio (1 -> 4/5)
           float datarate = sf * ((cr_) / (std::pow(2, sf) / b)); //Calculate the datarate given the sf bw and cr.
           
            // Remember that J = Watts x Time (seconds)
           //Calculate the TOA for a 1000 byte packet, and multiply it by the dBm -> W value. This will give you the Joules.
           return ((bits / datarate) * (std::pow(10, (p/10)) / 1000.0)) * 1000.0; //the 1000 at the end converts this to milliJoules. 
       }

        float TransmissionParameterSet::PowerConsumption(uint8_t spreadingfactor, uint32_t bandwidth, int codingrate, float power)
        {

            //First, calculate the data rate of the given parameters.
            double datarate = (spreadingfactor * (bandwidth * 4 / std::pow(2, spreadingfactor)) * (1.0f / (codingrate + 4.0f)));

            double highest_datarate = (12 * (125000 * 4 / std::pow(2, 12)) * (1.0f / (4 + 4.0f)));
            double highest_powerconsumption = (1000.0f / highest_datarate) * 16;

            double lowest_datarate = (7 * (250000 * 4 / std::pow(2, 7)) * (1.0f / (1 + 4.0f)));
            double lowest_powerconsumption = (1000.0f / lowest_datarate) * 2;
            //DataRate * Power
            //https://www.ncbi.nlm.nih.gov/pmc/articles/PMC7070984/
            //this function will calculate the amount of power needed to transmit 1000 bytes of data.
            //1000.0f / datarate gives us the amount of time, in seconds, it will take the phy to transmit 1000 bits of data.
            //1000 is chosen so that values are large enough such that they do not need to be expressed in scientific notation.
            double powerconsumption = (1000.0f / datarate) * power;

            //return powerconsumption;
            //Output 0-1 depending on where the power factor is between the theoretical min and max.
            //lowest  = 0.0914286
            //highest = 87.3813
            //(v - lowest) / (highest - lowest)
            ///return 1.0f - (powerconsumption - 0.0914286) / (87.3813 - 0.0914286); // - (-0.2 * (!succesful));
            return (powerconsumption - lowest_powerconsumption) / (highest_powerconsumption - lowest_powerconsumption);
        }

    }
}