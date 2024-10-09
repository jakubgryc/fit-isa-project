/**
 * @file FlowCache.h
 * @brief FlowCache header file
 * @author Jakub Gryc <xgrycj03>
 */

#ifndef FLOWCACHE_H
#define FLOWCACHE_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <arpa/inet.h>

#include "Flow.h"

/**
 * @class FlowCache
 * @brief Class representing cache of flows
 *
 * The Flow Cache class stores the individual flows in a hashmap.
 */
class FlowCache {
   public:
    void updateFlow(const Flow &flow, uint32_t packetSize);
    void removeFlow(const std::string &key);
    void print();

   private:
    std::unordered_map<std::string, std::shared_ptr<Flow>> flowCache;

    std::string getFlowKey(const Flow &flow);
};

#endif
