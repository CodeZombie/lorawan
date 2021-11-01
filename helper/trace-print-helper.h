/*
    This class abstracts away the process of hooking up trace source and trace sinks.
    It's primary goal is to take in a bunch of Nodes and output a bunch of files with trace data.
*/

#ifndef TRACE_PRINT_HELPER_H
#define TRACE_PRINT_HELPER_H

#include "config.h"
#include <vector>
#include <fstream>
#include "ns3/lora-helper.h"
#include "ns3/basic-energy-source-helper.h"
#include "ns3/lora-radio-energy-model-helper.h"
#include "ns3/value-watcher.h"



namespace ns3 {
namespace lorawan {

class TracePrintHelper {
    public:
        TracePrintHelper(Time updateInterval);
        void AddValueWatcher(ValueWatcher* watcher);
        void Start();
    private:
        /* Get's called every <interval> */
        static void update(std::vector<ValueWatcher*>* valueWatchers, Time updateInterval);

        /* The string-identifies of attributes to watch inside the Mac Layer of the end-nodes */
        std::vector<ValueWatcher*>* valueWatchers;

        //The prefix to use for all output files. (Use this to set a folder or something)
        std::string prefix;

        Time updateInterval;
};
}}

#endif