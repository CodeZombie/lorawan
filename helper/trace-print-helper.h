/*
    This class abstracts away the process of hooking up trace source and trace sinks.
    It's primary goal is to take in a bunch of Nodes and output a bunch of files with trace data.
*/

#ifndef TRACE_PRINT_HELPER_H
#define TRACE_PRINT_HELPER_H

#include "ns3/lora-helper.h"
#include <vector>
#include <fstream>

namespace ns3 {
namespace lorawan {
enum TracePrintAttributeTypes { Double, Integer, Uinteger };

class TracePrintAttribute {
    public:
        std::string name;
        enum TracePrintAttributeTypes type = TracePrintAttributeTypes::Double;
        bool average = false;
        std::ofstream fileStream;
};

class TracePrintHelper {

    public:
        TracePrintHelper(std::string prefix, NodeContainer* monitoredNodes, Time updateInterval);

        void WatchAttribute(std::string name, enum TracePrintAttributeTypes type, bool average);

    private:
        /* Get's called every <interval> */
        static void update(NodeContainer *monitoredNodes, std::vector<TracePrintAttribute*>* tracePrintAttributes, Time updateInterval);

        /* A pointer to the nodes inside the simulation that we want to monitor. */
        NodeContainer *monitoredNodes;

        /* The string-identifies of attributes to watch inside the Mac Layer of the end-nodes */
        std::vector<TracePrintAttribute*> tracePrintAttributes;

        std::string prefix;
};

}}

#endif