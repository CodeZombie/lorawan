#include "ns3/trace-print-helper.h"

namespace ns3
{
    namespace lorawan
    {

        TracePrintHelper::TracePrintHelper(Time updateInterval)
        {
            //set the schedular to call update();
            this->updateInterval = updateInterval;
            valueWatchers = new std::vector<ValueWatcher *>();
        }

        void TracePrintHelper::Start()
        {
            Simulator::Schedule(updateInterval, &(this->update), this->valueWatchers, this->updateInterval);
        }

        void TracePrintHelper::AddValueWatcher(ValueWatcher *watcher)
        {
            this->valueWatchers->push_back(watcher);
        }

        void TracePrintHelper::update(std::vector<ValueWatcher *> *valueWatchers, Time updateInterval)
        {
            for (auto watcher : *valueWatchers)
            {
                if (watcher->sourceType == ValueWatcher::SourceType::Attribute)
                {
                    watcher->ProbeAndSave();
                }
            }

            Simulator::Schedule(updateInterval, TracePrintHelper::update, valueWatchers, updateInterval);
        }

    }
}