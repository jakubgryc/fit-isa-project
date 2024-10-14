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
#include <queue>

#include "Flow.h"
#include "Tools.h"

/**
 * @class FlowCache
 * @brief Class representing cache of flows
 *
 * The Flow Cache class stores the individual flows in a hashmap.
 */
class FlowCache {
   public:
    
    FlowCache(Timer &timer);

    /**
     * @brief public function to update parameters of a flow such as timestamps and total packet size and count
     *
     * @param flow current flow
     * @param packetSize Packet size in bytes
     */
    void handleFlow(const Flow &flow, uint32_t packetSize, struct timeval packetTime);
    void removeFlow(const std::string &key);
    void flushToExportAll();
    void print();
    bool exportCacheFull();
    
    std::queue<struct NetflowRecord> &getExportCache();
    std::queue<struct NetflowRecord> exportCache;

   private:
    Timer timer;

    void checkForExpiredFlows(uint32_t timestamp);
    void prepareToExport(std::shared_ptr<Flow> flow);
    std::unordered_map<std::string, std::shared_ptr<Flow>> flowCache;
    std::string getFlowKey(const Flow &flow);
};

#endif
