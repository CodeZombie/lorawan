/*
    This class abstracts away the process of hooking up trace source and trace sinks.
    It's primary goal is to take in a bunch of Nodes and output a bunch of files with trace data.
*/

#ifndef TRACE_PRINT_HELPER_H
#define TRACE_PRINT_HELPER_H

#include "ns3/lora-helper.h"
#include "ns3/basic-energy-source-helper.h"
#include "ns3/lora-radio-energy-model-helper.h"
#include "config.h"
#include <vector>
#include <fstream>

namespace ns3 {
namespace lorawan {
enum TracePrintAttributeType { Double, Integer, Uinteger, Boolean };
enum TracePrintCombineMode {None, Average, Sum};


class AttributeWatcher {
    public:
        //The Constructor which takes an EnergySourceContainer.
        AttributeWatcher(std::string attributeName, ns3::DeviceEnergyModelContainer *nodeContainer, enum TracePrintAttributeType type, enum TracePrintCombineMode mode, std::string prefix) {
            this->attributeName = attributeName;
            this->type = type;
            this->mode = mode;
            for(auto node = nodeContainer->Begin(); node != nodeContainer->End(); node++) {
                this->watchedObjects.push_back(*node);
            }
            this->fileStream.open(prefix + attributeName + ".dat");
        }

        //The construct which takes a generic NodeContainer.
        AttributeWatcher(std::string attributeName, ns3::NodeContainer *nodeContainer, enum TracePrintAttributeType type, enum TracePrintCombineMode mode, std::string prefix) {
            this->attributeName = attributeName;
            this->type = type;
            this->mode = mode;
            for(auto node = nodeContainer->Begin(); node != nodeContainer->End(); node++) {
                this->watchedObjects.push_back(*node);
            }
            this->fileStream.open(prefix + attributeName + ".dat");
        }

        //The construct which takes a device path.
        AttributeWatcher(std::string attributeName, std::string path, enum TracePrintAttributeType type, enum TracePrintCombineMode mode, std::string prefix) {
            this->attributeName = attributeName;
            this->type = type;
            this->mode = mode;
            Config::MatchContainer m = Config::LookupMatches(path);
            for (Config::MatchContainer::Iterator i = m.Begin(); i != m.End(); ++i)
            {
                this->watchedObjects.push_back(*i);
            }
            this->fileStream.open(prefix + attributeName + ".dat");
        }

        void addWatchedObject(Ptr<Object> object) {
            this->watchedObjects.push_back(object);
        }

        void ProbeAndSave() {
            if (this->type == TracePrintAttributeType::Double) {
                double sum = 0;
                for (Ptr<Object> object : this->watchedObjects) {
                    DoubleValue value = 0;
                    object->GetAttribute(this->attributeName, value);
                    if(this->mode == TracePrintCombineMode::None) {             //Save to file immediately.
                        this->fileStream << ns3::Simulator::Now().GetHours() << " " << value.Get() << std::endl;
                    }
                    sum += value.Get();
                }
                if(this->mode == TracePrintCombineMode::Average) {    //Store all values in sum and divide by number of objects.
                    this->fileStream << ns3::Simulator::Now().GetHours() << " " << ((float)sum / (float)this->watchedObjects.size()) << std::endl;
                }else if(this->mode == TracePrintCombineMode::Sum) {        //Store all values in sum but don't divide.
                    this->fileStream << ns3::Simulator::Now().GetHours() << " " << sum << std::endl;
                }
            }else if(this->type == TracePrintAttributeType::Integer) {
                int sum = 0;
                for (Ptr<Object> object : this->watchedObjects) {
                    IntegerValue value = 0;
                    
                    object->GetAttribute(this->attributeName, value);
                    if(this->mode == TracePrintCombineMode::None) {             //Save to file immediately.
                        this->fileStream << ns3::Simulator::Now().GetHours() << " " << value.Get() << std::endl;
                    }
                    sum += value.Get();
                }
                if(this->mode == TracePrintCombineMode::Average) {    //Store all values in sum and divide by number of objects.
                    this->fileStream << ns3::Simulator::Now().GetHours() << " " << ((float)sum / (float)this->watchedObjects.size()) << std::endl;
                }else if(this->mode == TracePrintCombineMode::Sum) {        //Store all values in sum but don't divide.
                    this->fileStream << ns3::Simulator::Now().GetHours() << " " << sum << std::endl;
                }
            }else if(this->type == TracePrintAttributeType::Uinteger) {
                uint32_t  sum = 0;
                for (Ptr<Object> object : this->watchedObjects) {
                    UintegerValue value = 0;
                    object->GetAttribute(this->attributeName, value);
                    if(this->mode == TracePrintCombineMode::None) {             //Save to file immediately.
                        this->fileStream << ns3::Simulator::Now().GetHours() << " " << value.Get() << std::endl;
                    }
                    sum += value.Get();
                }
                if(this->mode == TracePrintCombineMode::Average) {    //Store all values in sum and divide by number of objects.
                    this->fileStream << ns3::Simulator::Now().GetHours() << " " << ((float)sum / (float)this->watchedObjects.size()) << std::endl;
                }else if(this->mode == TracePrintCombineMode::Sum) {        //Store all values in sum but don't divide.
                    this->fileStream << ns3::Simulator::Now().GetHours() << " " << sum << std::endl;
                }
            }
        }

        std::string attributeName;
        enum TracePrintAttributeType type;
        enum TracePrintCombineMode mode;
        std::vector<ns3::Ptr<ns3::Object>> watchedObjects;
        std::ofstream fileStream;
};

class TracePrintHelper {
    public:
        TracePrintHelper(Time updateInterval);
        void AddAttributeWatcher(AttributeWatcher* watcher);
        void Start();
    private:
        /* Get's called every <interval> */
        static void update(std::vector<AttributeWatcher*> attributeWatchers, Time updateInterval);

        /* The string-identifies of attributes to watch inside the Mac Layer of the end-nodes */
        std::vector<AttributeWatcher*> attributeWatchers;

        //The prefix to use for all output files. (Use this to set a folder or something)
        std::string prefix;

        Time updateInterval;
};
}}

#endif