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
    

    /**
     * @brief flushes the flow cache to export cache
     *
     */
    void flushToExportAll();

    
    /**
     * @brief checks if the export cache is full
     *
     * @return true if the export cache is full (30 or more records), false otherwise
     */
    bool exportCacheFull();
    
    /**
     * @brief returns the export cache
     *
     * @return reference to the export cache
     */
    std::queue<struct NetflowRecord> &getExportCache();

    /**
     * @brief returns the flow cache
     *
     * @return reference to the flow cache
     */
    std::queue<struct NetflowRecord> exportCache;

   private:
    
    Timer timer;

    /**
     * @brief loops through the flow cache and checks if any flow has expired, if so, it sends it to the export cache
     *
     * @param timestamp current timestamp
     */
    void checkForExpiredFlows(uint32_t timestamp);

    /**
     * @brief prepares the flow to be exported
     *
     * @param flow current flow
     */
    void prepareToExport(std::shared_ptr<Flow> flow);

    /**
     * @brief returns the key of the flow from the source and destination IPs and ports
     *
     * @param flow current flow
     * @return key of the flow
     */
    std::string getFlowKey(const Flow &flow);
    std::unordered_map<std::string, std::shared_ptr<Flow>> flowCache;
};

#endif
