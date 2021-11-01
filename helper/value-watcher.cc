#include "ns3/value-watcher.h"

namespace ns3
{
    namespace lorawan
    {

        //The Constructor which takes an EnergySourceContainer.
        ValueWatcher::ValueWatcher(std::string attributeName,
                                   ns3::DeviceEnergyModelContainer *nodeContainer,
                                   enum ValueWatcher::Type type,
                                   enum ValueWatcher::CombineMode mode,
                                   enum ValueWatcher::SourceType sourceType, std::string prefix)
        {
            this->attributeName = attributeName;
            this->type = type;
            this->mode = mode;
            this->sourceType = sourceType;
            for (auto node = nodeContainer->Begin(); node != nodeContainer->End(); node++)
            {
                this->watchedObjects.push_back(*node);
            }
            this->fileStream.open(prefix + attributeName + ".dat");
        }

        //The construct which takes a generic NodeContainer.
        ValueWatcher::ValueWatcher(std::string attributeName,
                                   ns3::NodeContainer *nodeContainer,
                                   enum ValueWatcher::Type type,
                                   enum ValueWatcher::CombineMode mode,
                                   enum ValueWatcher::SourceType sourceType,
                                   std::string prefix)
        {
            this->attributeName = attributeName;
            this->type = type;
            this->mode = mode;
            this->sourceType = sourceType;
            for (auto node = nodeContainer->Begin(); node != nodeContainer->End(); node++)
            {
                this->watchedObjects.push_back(*node);
            }
            this->fileStream.open(prefix + attributeName + ".dat");
        }

        //The construct which takes a device path.
        ValueWatcher::ValueWatcher(std::string attributeName,
                                   std::string path, enum ValueWatcher::Type type, enum ValueWatcher::CombineMode mode,
                                   enum ValueWatcher::SourceType sourceType,
                                   std::string prefix)
        {
            this->attributeName = attributeName;
            this->type = type;
            this->mode = mode;
            this->sourceType = sourceType;
            ns3::Config::MatchContainer m = ns3::Config::LookupMatches(path);
            if (sourceType == Attribute)
            {
                for (ns3::Config::MatchContainer::Iterator i = m.Begin(); i != m.End(); ++i)
                {
                    this->watchedObjects.push_back(*i);
                }
            }
            else if (sourceType == ValueWatcher::SourceType::TraceSource)
            {
                if (type == Integer || type == Uinteger)
                {
                    ns3::Config::ConnectWithoutContext(path + "/" + attributeName, MakeCallback(&ValueWatcher::StoreIntegerValue, this));
                }
                else if (type == Double)
                {
                    ns3::Config::ConnectWithoutContext(path + "/" + attributeName, MakeCallback(&ValueWatcher::StoreIntegerValue, this));
                }
            }

            this->fileStream.open(prefix + attributeName + ".dat");
        }

        void ValueWatcher::StoreIntegerValue(int old_value, int new_value)
        {
            int val = new_value;
            if(this->mode == Sum){
                this->TraceSourceSumInteger += (new_value - old_value);
                val = this->TraceSourceSumInteger;
            }
            this->fileStream << ns3::Simulator::Now().GetHours() << " " << val << std::endl;
        }

        void ValueWatcher::StoreDoubleValue(double old_value, double new_value)
        {
            this->fileStream << ns3::Simulator::Now().GetHours() << " " << new_value << std::endl;
        }

        void ValueWatcher::ProbeAndSave()
        {
            if(this->sourceType == ValueWatcher::SourceType::TraceSource){
                return;
            }

            if (this->type == ValueWatcher::Type::Double)
            {
                double sum = 0;
                for (Ptr<Object> object : this->watchedObjects)
                {
                    DoubleValue value = 0;
                    object->GetAttribute(this->attributeName, value);
                    if (this->mode == ValueWatcher::CombineMode::None)
                    { //Save to file immediately.
                        this->fileStream << ns3::Simulator::Now().GetHours() << " " << value.Get() << std::endl;
                    }
                    sum += value.Get();
                }
                if (this->mode == ValueWatcher::CombineMode::Average)
                { //Store all values in sum and divide by number of objects.
                    this->fileStream << ns3::Simulator::Now().GetHours() << " " << ((float)sum / (float)this->watchedObjects.size()) << std::endl;
                }
                else if (this->mode == ValueWatcher::CombineMode::Sum)
                { //Store all values in sum but don't divide.
                    this->fileStream << ns3::Simulator::Now().GetHours() << " " << sum << std::endl;
                }
            }
            else if (this->type == ValueWatcher::Type::Integer)
            {
                int sum = 0;
                for (Ptr<Object> object : this->watchedObjects)
                {
                    IntegerValue value = 0;

                    object->GetAttribute(this->attributeName, value);
                    if (this->mode == ValueWatcher::CombineMode::None)
                    { //Save to file immediately.
                        this->fileStream << ns3::Simulator::Now().GetHours() << " " << value.Get() << std::endl;
                    }
                    sum += value.Get();
                }
                if (this->mode == ValueWatcher::CombineMode::Average)
                { //Store all values in sum and divide by number of objects.
                    this->fileStream << ns3::Simulator::Now().GetHours() << " " << ((float)sum / (float)this->watchedObjects.size()) << std::endl;
                }
                else if (this->mode == ValueWatcher::CombineMode::Sum)
                { //Store all values in sum but don't divide.
                    this->fileStream << ns3::Simulator::Now().GetHours() << " " << sum << std::endl;
                }
            }
            else if (this->type == ValueWatcher::Type::Uinteger)
            {
                uint32_t sum = 0;
                for (Ptr<Object> object : this->watchedObjects)
                {
                    UintegerValue value = 0;
                    object->GetAttribute(this->attributeName, value);
                    if (this->mode == ValueWatcher::CombineMode::None)
                    { //Save to file immediately.
                        this->fileStream << ns3::Simulator::Now().GetHours() << " " << value.Get() << std::endl;
                    }
                    sum += value.Get();
                }
                if (this->mode == ValueWatcher::CombineMode::Average)
                { //Store all values in sum and divide by number of objects.
                    this->fileStream << ns3::Simulator::Now().GetHours() << " " << ((float)sum / (float)this->watchedObjects.size()) << std::endl;
                }
                else if (this->mode == ValueWatcher::CombineMode::Sum)
                { //Store all values in sum but don't divide.
                    this->fileStream << ns3::Simulator::Now().GetHours() << " " << sum << std::endl;
                }
            }
            else if (this->type == ValueWatcher::Type::Enum)
            {
                for (Ptr<Object> object : this->watchedObjects)
                {
                    EnumValue value = 0;
                    object->GetAttribute(this->attributeName, value);
                    this->fileStream << ns3::Simulator::Now().GetHours() << " " << value.Get() << std::endl;
                }
            }
        }
    }
}
