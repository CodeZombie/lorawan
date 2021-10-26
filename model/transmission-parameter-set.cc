#include "ns3/transmission-parameter-set.h"
//TODO:
//      Refine the valid ranges of data. For example, TXp 14 and BW 500k I think is invalid.
//      The system should check for that.
namespace ns3
{
    namespace lorawan
    {
        
        NS_LOG_COMPONENT_DEFINE("TransmissionParameterSet");

        TransmissionParameterSet::TransmissionParameterSet()
        {
            initializeRNG();
            //spreading factor: 7 to 12
            spreadingFactor = randomGenerator->GetInteger(7, 12);
            //power: 2, 4, 6, 8, 10, 12, 14, 16
            power = randomGenerator->GetInteger(1, 7) * 2; //8
            //bandwidth: 125000, 250000, 500000
            int choice = randomGenerator->GetInteger(1, 3); /// 2????
            if (choice == 1)
            {
                bandwidth = 125000;
            }
            else if (choice == 2)
            {
                bandwidth = 250000;
            }
            else
            {
                bandwidth = 500000;
            }
            //coding rate: 1 (4/5), 2 (4/6), 3 (4/7), 4 (4/8)
            codingRate = randomGenerator->GetInteger(1, 4);
        }

        bool TransmissionParameterSet::isEqual(TransmissionParameterSet *other)
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

        TransmissionParameterSet::TransmissionParameterSet(TransmissionParameterSet* other) {
            initializeRNG();
            spreadingFactor = other->spreadingFactor;
            power = other->power;
            bandwidth = other->bandwidth;
            codingRate = other->codingRate;
        }

        TransmissionParameterSet::TransmissionParameterSet(TransmissionParameterSet *parent_a, TransmissionParameterSet *parent_b)
        {
            initializeRNG();
            //generate parameters based on parent's parameters.
            int sf_choice = randomGenerator->GetInteger(1, 2);
            if (sf_choice == 1)
            {
                spreadingFactor = parent_a->spreadingFactor;
            }
            else
            {
                spreadingFactor = parent_b->spreadingFactor;
            }

            int pow_choice = randomGenerator->GetInteger(1, 2);
            if (pow_choice == 1)
            {
                power = parent_a->power;
            }
            else
            {
                power = parent_b->power;
            }

            int bw_choice = randomGenerator->GetInteger(1, 2);
            if (bw_choice == 1)
            {
                bandwidth = parent_a->bandwidth;
            }
            else
            {
                bandwidth = parent_b->bandwidth;
            }

            int cr_choice = randomGenerator->GetInteger(1, 2);
            if (cr_choice == 1)
            {
                codingRate = parent_a->codingRate;
            }
            else
            {
                codingRate = parent_b->codingRate;
            }

            if (randomGenerator->GetInteger(1, 100) > 75)
            {
                int mutate_choice = randomGenerator->GetInteger(0, 3);
                if (mutate_choice == 0)
                {
                    //mutate SF
                    spreadingFactor = mutateValue(spreadingFactor, 1, 7, 12);
                }
                else if (mutate_choice == 1)
                {
                    //mutate power
                    power = mutateValue(power, 2, 2, 14);
                }
                else if (mutate_choice == 2)
                {
                    //mutate cr
                    codingRate = mutateValue(codingRate, 1, 1, 4);
                }
                else
                {
                    //mutate bw
                    if (bandwidth == 125000)
                    {
                        bandwidth *= 2;
                    }
                    else if (bandwidth == 500000)
                    {
                        bandwidth /= 2;
                    }
                    else
                    { //bandwidth == 250000
                        int mutate_direction = randomGenerator->GetInteger(0, 1);
                        if (mutate_direction == 0)
                        {
                            bandwidth *= 2;
                        }
                        else
                        {
                            bandwidth /= 2;
                        }
                    }
                }
            }
        }
        
        float TransmissionParameterSet::getPER() {
            int totalTransmissions = successCount + failureCount;
            if(totalTransmissions == 0){
                return 0.0;
            }
            return 1 - ((float)failureCount / (float)totalTransmissions);
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
            std::cout << "TXPARAMS: SF=" << spreadingFactor << " PW=" << power << " BW=" << bandwidth << " CR=" << codingRate << " FITNESS=" << fitness() << " PER=" << getPER() << std::endl;
        }
        
        void TransmissionParameterSet::onAckOrNack(bool successful) {
            if(successful){
                successCount++;
            }else{
                failureCount++;
            }
        }

        bool TransmissionParameterSet::CompareFitness(TransmissionParameterSet *a, TransmissionParameterSet *b) {
            return a->fitness() > b->fitness();
        }

        //  Fitness function: lower is better.
        float TransmissionParameterSet::fitness()
        {
            float powerConsumption = PowerConsumption(this->spreadingFactor, this->bandwidth, this->codingRate, this->power);
            return powerConsumption + (powerConsumption * getPER());
        }



        float TransmissionParameterSet::PowerConsumption(uint8_t spreadingfactor, uint32_t bandwidth, int codingrate, float power)
        {
            //TODO: incorporate PER into this.



            //First, calculate the data rate of the given parameters.
            float datarate = (spreadingfactor * (bandwidth * 4 / std::pow(2, spreadingfactor)) * (1.0f / (codingrate + 4.0f)));

            //DataRate * Power
            //https://www.ncbi.nlm.nih.gov/pmc/articles/PMC7070984/
            //this function will calculate the amount of power needed to transmit 1000 bytes of data.
            //1000 is chosen so that values are large enough such that they do not need to be expressed in scientific notation.
            float powerconsumption = (1000.0f / datarate) * power;

            //Output 0-1 depending on where the power factor is between the theoretical min and max.
            //lowest  = 0.0914286
            //highest = 87.3813
            //(v - lowest) / (highest - lowest)
            return 1.0f - (powerconsumption - 0.0914286) / (87.3813 - 0.0914286); // - (-0.2 * (!succesful));
        }

    }
}