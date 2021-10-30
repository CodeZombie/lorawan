#include "ns3/trace-print-helper.h"

namespace ns3 {
namespace lorawan {

    TracePrintHelper::TracePrintHelper(Time updateInterval) {
        //set the schedular to call update();
        this->updateInterval = updateInterval;
        attributeWatchers = new std::vector<AttributeWatcher*>();
    }

    void TracePrintHelper::Start() {
        Simulator::Schedule(updateInterval, &(this->update), this->attributeWatchers, this->updateInterval);
    }

    void TracePrintHelper::AddAttributeWatcher(AttributeWatcher* watcher) {
        this->attributeWatchers->push_back(watcher);
    }


    void TracePrintHelper::update(std::vector<AttributeWatcher*> *attributeWatchers, Time updateInterval) {
        for(auto watcher : *attributeWatchers) {
            watcher->ProbeAndSave();
        }

        Simulator::Schedule(updateInterval, TracePrintHelper::update, attributeWatchers, updateInterval);
    }

}}