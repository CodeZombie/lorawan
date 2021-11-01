/* Written by Jeremy Clark

This class is used to watch for changes in a value, and prints the value to a stream with associated time.
This is the basis of all data generation for the experiment it was created for.
One instance of this class will be created for each Trace Source/Attribute that needs to be watched.
The instance will keep track of and/or find all instances of objects that are being watched.
*/
#include "device-energy-model-container.h"
#include "ns3/core-module.h"
#include "node-container.h"
#include "ns3/simulator.h"
#include "ns3/config-store-module.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <string>

#ifndef VALUE_WATCHER_H
#define VALUE_WATCHER_H
namespace ns3
{
    namespace lorawan
    {
        class ValueWatcher
        {
            public:
            enum Type
            {
                Double,
                Integer,
                Uinteger,
                Boolean,
                Enum
            };
            enum CombineMode
            {
                None,
                Average,
                Sum
            };

            enum SourceType
            {
                Attribute,  //Need to be polled.
                TraceSource //Value read from callback (instant, no polling)
            };

            //The Constructor which takes an EnergySourceContainer.
            ValueWatcher(std::string attributeName, ns3::DeviceEnergyModelContainer *nodeContainer, enum Type type, enum CombineMode mode, enum SourceType sourceType, std::string prefix);

            //The construct which takes a generic NodeContainer.
            ValueWatcher(std::string attributeName, ns3::NodeContainer *nodeContainer, enum Type type, enum CombineMode mode, enum SourceType sourceType, std::string prefix);
            //The construct which takes a device path.
            ValueWatcher(std::string attributeName, std::string path, enum Type type, enum CombineMode mode, enum SourceType sourceType, std::string prefix);

            void StoreIntegerValue(int old_value, int new_value);

            void StoreDoubleValue(double old_value, double new_value);

            void ProbeAndSave();

            std::string attributeName;
            enum Type type;
            enum CombineMode mode;
            enum SourceType sourceType;
            std::vector<ns3::Ptr<ns3::Object>> watchedObjects;
            std::ofstream fileStream;
            int32_t TraceSourceSumInteger = 0;
            double TraceSourceSumDouble = 0;
        };
    }
}
#endif