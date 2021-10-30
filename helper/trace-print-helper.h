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
enum TracePrintAttributeTypes { Double, Integer, Uinteger, Boolean };
enum TracePrintCombineMode {None, Average, Sum};
enum TracePrintAttributeLocation {MAC, EnergyModel};

class TracePrintAttribute {
    public:
        std::string name;
        enum TracePrintAttributeTypes type = TracePrintAttributeTypes::Double;
        enum TracePrintCombineMode mode;
        enum TracePrintAttributeLocation location;
        std::ofstream fileStream;
};

class AttributeWatcher {
    public:
        //The Constructor which takes an EnergySourceContainer.
        AttributeWatcher(std::string attributeName, ns3::EnergySourceContainer *nodeContainer, enum TracePrintAttributeTypes type, enum TracePrintCombineMode mode, std::string prefix) {
            this->attributeName = attributeName;
            this->type = type;
            this->mode = mode;
            for(auto node = nodeContainer->Begin(); node != nodeContainer->End(); node++) {
                this->watchedObjects.push_back(*node);
            }
            this->fileStream.open(prefix + attributeName + ".dat");
        }

        //The construct which takes a generic NodeContainer.
        AttributeWatcher(std::string attributeName, ns3::NodeContainer *nodeContainer, enum TracePrintAttributeTypes type, enum TracePrintCombineMode mode, std::string prefix) {
            this->attributeName = attributeName;
            this->type = type;
            this->mode = mode;
            for(auto node = nodeContainer->Begin(); node != nodeContainer->End(); node++) {
                this->watchedObjects.push_back(*node);
            }
            this->fileStream.open(prefix + attributeName + ".dat");
        }

        //The construct which takes a device path.
        AttributeWatcher(std::string attributeName, std::string path, enum TracePrintAttributeTypes type, enum TracePrintCombineMode mode, std::string prefix) {
            this->attributeName = attributeName;
            this->type = type;
            this->mode = mode;
            Config::MatchContainer m = Config::LookupMatches(path); // "/NodeList/*/DeviceList/*/$ns3::LoraNetDevice/Mac"
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
            if (this->type == TracePrintAttributeTypes::Double) {
                double sum = 0;
                for (Ptr<Object> object : this->watchedObjects) {
                    DoubleValue value = 0;
                    object->GetAttribute(this->attributeName, value);
                    if(this->mode == TracePrintCombineMode::None) {             //Save to file immediately.
                        this->fileStream << ns3::Simulator::Now().GetMinutes() << " " << value.Get() << std::endl;
                    }
                    sum += value.Get();
                }
                if(this->mode == TracePrintCombineMode::Average) {    //Store all values in sum and divide by number of objects.
                    this->fileStream << ns3::Simulator::Now().GetMinutes() << " " << ((float)sum / (float)this->watchedObjects.size()) << std::endl;
                }else if(this->mode == TracePrintCombineMode::Sum) {        //Store all values in sum but don't divide.
                    this->fileStream << ns3::Simulator::Now().GetMinutes() << " " << sum << std::endl;
                }
            }
        }

        std::string attributeName;
        enum TracePrintAttributeTypes type;
        enum TracePrintCombineMode mode;
        std::vector<ns3::Ptr<ns3::Object>> watchedObjects;
        std::ofstream fileStream;
};

class TracePrintTraceSource {
    public:
    std::string path; //"/Names/EnergySource/RemainingEnergy"
    enum TracePrintAttributeTypes type;
    enum TracePrintCombineMode combineMode;
    std::ofstream fileStream;
    double doubleCombinedValue;
    int intCombinedValue;
    void RegisterCallback(ns3::Ptr<ns3::Object> o) {
        o->TraceConnectWithoutContext(path, MakeCallback(&TracePrintTraceSource::callback, this));
    }
    void callback(double a, double b) {
        std::cout << a << " " << b << std::endl;
    }
};

class TracePrintHelper {
    public:
        TracePrintHelper(std::string prefix, NodeContainer* monitoredNodes, Time updateInterval);
        void WatchAttribute(std::string name, enum TracePrintAttributeTypes type, enum TracePrintAttributeLocation location, enum TracePrintCombineMode mode);
        TracePrintTraceSource* ConnectTraceSource(std::string path, enum TracePrintAttributeTypes type, enum TracePrintCombineMode combineMode);

        void AddAttributeWatcher(AttributeWatcher* watcher);
    private:
        /* Get's called every <interval> */
        static void update(NodeContainer *monitoredNodes, std::vector<TracePrintAttribute*>* tracePrintAttributes, Time updateInterval);

        /* A pointer to the nodes inside the simulation that we want to monitor. */
        NodeContainer *monitoredNodes;

        /* The string-identifies of attributes to watch inside the Mac Layer of the end-nodes */
        std::vector<TracePrintAttribute*> tracePrintAttributes;
        std::vector<TracePrintTraceSource*> tracePrintTraceSources;
        std::string prefix;


        std::vector<AttributeWatcher*> attributeWatchers;
};

}}

#endif