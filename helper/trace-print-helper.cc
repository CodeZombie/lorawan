#include "ns3/trace-print-helper.h"

namespace ns3 {
namespace lorawan {

    TracePrintHelper::TracePrintHelper(std::string prefix, NodeContainer* monitoredNodes, Time updateInterval) {
        this->monitoredNodes = monitoredNodes;
        this->prefix = prefix;
        //set the schedular to call update();
        Simulator::Schedule(updateInterval, &(this->update), this->monitoredNodes, &this->tracePrintAttributes, updateInterval);
    }

    void TracePrintHelper::WatchAttribute(std::string name, enum TracePrintAttributeTypes type, bool average = false) {
        TracePrintAttribute *tracePrintAttribute = new TracePrintAttribute();
        tracePrintAttribute->name = name;
        tracePrintAttribute->type = type;
        tracePrintAttribute->average = average;
        tracePrintAttribute->fileStream.open(this->prefix + name + ".dat");

        this->tracePrintAttributes.push_back(tracePrintAttribute);
    }

    //Dont think casting ints directly to doubles is gonna work. Lets check tho
    void TracePrintHelper::update(NodeContainer *monitoredNodes, std::vector<TracePrintAttribute*>* tracePrintAttributes, Time updateInterval) {
        for (std::vector<TracePrintAttribute*>::iterator attribute = tracePrintAttributes->begin(); attribute != tracePrintAttributes->end(); ++attribute) {
            for (NodeContainer::Iterator j = monitoredNodes->Begin (); j != monitoredNodes->End (); ++j) {
                if((*attribute)->type == TracePrintAttributeTypes::Double) {
                    DoubleValue value;
                    (*j)->GetDevice(0)->GetObject<LoraNetDevice>()->GetMac()->GetAttribute((*attribute)->name, value); 
                    (*attribute)->fileStream << ns3::Simulator::Now().GetHours() << " " << value.Get() << std::endl;
                }else if((*attribute)->type == TracePrintAttributeTypes::Integer){
                    IntegerValue value;
                    (*j)->GetDevice(0)->GetObject<LoraNetDevice>()->GetMac()->GetAttribute((*attribute)->name, value); 
                    (*attribute)->fileStream << ns3::Simulator::Now().GetHours() << " " << value.Get() << std::endl;
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
        }

        Simulator::Schedule(updateInterval, TracePrintHelper::update, monitoredNodes, tracePrintAttributes, updateInterval);
    }

}}