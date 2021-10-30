#include "ns3/trace-print-helper.h"

namespace ns3 {
namespace lorawan {
    void PrintEnergyRemaining(double a, double b){
        std::cout << "WOW: " << a << " " << b << std::endl;
    }
    TracePrintHelper::TracePrintHelper(std::string prefix, NodeContainer* monitoredNodes, Time updateInterval) {
        this->monitoredNodes = monitoredNodes;
        this->prefix = prefix;
        //set the schedular to call update();
        Simulator::Schedule(updateInterval, &(this->update), this->monitoredNodes, &this->tracePrintAttributes, updateInterval);
    }

    void TracePrintHelper::WatchAttribute(std::string name, enum TracePrintAttributeTypes type, enum TracePrintAttributeLocation location, enum TracePrintCombineMode mode) {
        TracePrintAttribute *tracePrintAttribute = new TracePrintAttribute();
        tracePrintAttribute->name = name;
        tracePrintAttribute->type = type;
        tracePrintAttribute->mode = mode;
        tracePrintAttribute->location = location;
        
        tracePrintAttribute->fileStream.open(this->prefix + name + ".dat");
        this->tracePrintAttributes.push_back(tracePrintAttribute);
    }


    TracePrintTraceSource* TracePrintHelper::ConnectTraceSource(std::string path, enum TracePrintAttributeTypes type, enum TracePrintCombineMode combineMode) {
        TracePrintTraceSource *tracePrintTraceSource = new TracePrintTraceSource();
        tracePrintTraceSource->path = path;
        tracePrintTraceSource->type = type;
        tracePrintTraceSource->combineMode = combineMode;
        this->tracePrintTraceSources.push_back(tracePrintTraceSource);
        return tracePrintTraceSource;
    }

    void TracePrintHelper::AddAttributeWatcher(AttributeWatcher* watcher) {
        this->attributeWatchers.push_back(watcher);
    }


    void TracePrintHelper::update(NodeContainer *monitoredNodes, std::vector<TracePrintAttribute*>* tracePrintAttributes, Time updateInterval) {
        for(auto watcher : this->attributeWatchers) {
            watcher->ProbeAndSave();
        }


        return;
        for (std::vector<TracePrintAttribute*>::iterator attribute = tracePrintAttributes->begin(); attribute != tracePrintAttributes->end(); ++attribute) {
            double doubleSumValue = 0;
            uint32_t intSumValue = 0;

            for (NodeContainer::Iterator j = monitoredNodes->Begin (); j != monitoredNodes->End (); ++j) {
                if((*attribute)->type == TracePrintAttributeTypes::Double) {
                    DoubleValue value;
                    if((*attribute)->location == TracePrintAttributeLocation::MAC){
                        (*j)->GetDevice(0)->GetObject<LoraNetDevice>()->GetMac()->GetAttribute((*attribute)->name, value); 
                    }else if((*attribute)->location == TracePrintAttributeLocation::EnergyModel){
                        std::cout << (*j)->GetDevice(0)->GetObject<LoraRadioEnergyModel>() << std::endl;
                        (*j)->GetDevice(0)->GetObject<LoraRadioEnergyModel>()->GetAttribute((*attribute)->name, value);
                    }
                    if((*attribute)->mode == TracePrintCombineMode::Sum) {
                        doubleSumValue += value.Get();
                    }else{
                        (*attribute)->fileStream << ns3::Simulator::Now().GetHours() << " " << value.Get() << std::endl;
                    }
                }else if((*attribute)->type == TracePrintAttributeTypes::Integer){
                    IntegerValue value;
                    (*j)->GetDevice(0)->GetObject<LoraNetDevice>()->GetMac()->GetAttribute((*attribute)->name, value); 
                    if((*attribute)->mode == TracePrintCombineMode::Sum) {
                        intSumValue += value.Get();
                    }else{
                        (*attribute)->fileStream << ns3::Simulator::Now().GetHours() << " " << value.Get() << std::endl;
                    }
                }else if((*attribute)->type == TracePrintAttributeTypes::Uinteger){
                    UintegerValue value;
                    (*j)->GetDevice(0)->GetObject<LoraNetDevice>()->GetMac()->GetAttribute((*attribute)->name, value); 
                    (*attribute)->fileStream << ns3::Simulator::Now().GetHours() << " " << value.Get() << std::endl;
                }else if((*attribute)->type == TracePrintAttributeTypes::Boolean){
                    BooleanValue value;
                    (*j)->GetDevice(0)->GetObject<LoraNetDevice>()->GetMac()->GetAttribute((*attribute)->name, value); 
                    (*attribute)->fileStream << ns3::Simulator::Now().GetHours() << " " << value.Get() << std::endl;
                }
            }
            if((*attribute)->mode == TracePrintCombineMode::Sum){
                if((*attribute)->type == TracePrintAttributeTypes::Double){
                    (*attribute)->fileStream << ns3::Simulator::Now().GetHours() << " " << doubleSumValue << std::endl;
                }
                else if((*attribute)->type == TracePrintAttributeTypes::Integer){
                    (*attribute)->fileStream << ns3::Simulator::Now().GetHours() << " " << intSumValue << std::endl;
                }
            }
        }

        Simulator::Schedule(updateInterval, TracePrintHelper::update, monitoredNodes, tracePrintAttributes, updateInterval);
    }

}}