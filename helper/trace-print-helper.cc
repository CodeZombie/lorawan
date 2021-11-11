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

        double TracePrintHelper::GetDoubleValue(std::string name) {
            //for each valueWatcher in valueWatchers
            for (std::vector<ValueWatcher *>::iterator it = valueWatchers->begin(); it != valueWatchers->end(); ++it) {
                //if the name of the valueWatcher is equal to the name
                if ((*it)->attributeName == name) {
                    //return the value of the valueWatcher
                    return (*it)->TraceSourceSumDouble;
                }
            }
            return -1.0;
        }

        int TracePrintHelper::GetIntValue(std::string name) {
            //for each valueWatcher in valueWatchers
            for (std::vector<ValueWatcher *>::iterator it = valueWatchers->begin(); it != valueWatchers->end(); ++it) {
                //if the name of the valueWatcher is equal to the name
                if ((*it)->attributeName == name) {
                    //return the value of the valueWatcher
                    return (*it)->TraceSourceSumInteger;
                }
            }
            return -1;
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