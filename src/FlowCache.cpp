/**
 * @file FlowCache.cpp
 * @brief FlowCache implementation file
 * @author Jakub Gryc <xgrycj03>
 */



#include "../include/FlowCache.h"

#include <netinet/in.h>

#include <map>
#include <vector>

FlowCache::FlowCache(Timer &timer) : timer(timer) {}

void FlowCache::handleFlow(const Flow &flow, uint32_t packetSize, struct timeval packetTime) {
    
    uint32_t packetTimestamp = timer.getTimeDifference(&packetTime, timer.getStartTime());
    
    checkForExpiredFlows(packetTimestamp);

    std::string flowKey = getFlowKey(flow);

    auto it = flowCache.find(flowKey);

    if (it == flowCache.end()) {
        // Flow not in flowcache, create a new one
        flowCache[flowKey] = std::make_shared<Flow>(flow);
        flowCache[flowKey]->setFirst(packetTimestamp, flow.tcpFlags);
        flowCache[flowKey]->update(packetSize, packetTimestamp);
    } else {
        // Flow is already in flowcache, update its information
        it->second->update(packetSize, packetTimestamp);
        it->second->tcpFlags |= flow.tcpFlags;
    }

    return;
}

void FlowCache::prepareToExport(std::shared_ptr<Flow> flow) {
    struct NetflowRecord nfRecord;
    nfRecord.srcIP = flow->srcIP;    // already in network order
    nfRecord.destIP = flow->destIP;  // already in network order
    nfRecord.nexthop = htonl(0);
    nfRecord.SNMPinput = htons(0);
    nfRecord.SNMPoutput = htons(0);
    nfRecord.totalPackets = htonl(flow->packetCount);
    nfRecord.totalBytes = htonl(flow->byteCount);
    nfRecord.firstSeen = htonl(flow->startTime);
    nfRecord.lastSeen = htonl(flow->lastSeenTime);
    nfRecord.srcPort = flow->srcPort;    // already in network order
    nfRecord.destPort = flow->destPort;  // already in network order
    nfRecord.pad1 = 0;
    nfRecord.TCPflags = flow->tcpFlags;
    nfRecord.protocol = IPPROTO_TCP;
    nfRecord.tos = 0;
    nfRecord.srcAS = htons(0);
    nfRecord.destAS = htons(0);
    nfRecord.srcMask = 0;
    nfRecord.destMask = 0;
    nfRecord.pad2 = htons(0);

    exportCache.push(nfRecord);
}

void FlowCache::flushToExportAll() {
    std::map<uint32_t, std::vector<std::shared_ptr<Flow>>> exportMap;
    for (auto it = flowCache.begin(); it != flowCache.end();) {
        exportMap[it->second->startTime].push_back(it->second);
        it = flowCache.erase(it);
    }

    // Loop through the export map in descending order to export the flows with the oldest start time first
    for (auto it = exportMap.begin(); it != exportMap.end(); it++) {
        for (const auto &flow : it->second) {
            prepareToExport(flow);
        }
    }
}

void FlowCache::checkForExpiredFlows(uint32_t timestamp) {
    std::map<uint32_t, std::vector<std::shared_ptr<Flow>>> exportMap;
    uint32_t expirationTime;
    for (auto it = flowCache.begin(); it != flowCache.end();) {
        if (timer.checkFlowTimeouts(it->second->startTime, it->second->lastSeenTime, timestamp, &expirationTime)) {
            // Flow is expired, send it to export cache and remove from flow cache
            exportMap[expirationTime].push_back(it->second);
            it = flowCache.erase(it);
        } else {
            it++;
        }
    }

    // Loop through the export map in descending order to export the flows with the biggest expiration time first
    for (auto it = exportMap.rbegin(); it != exportMap.rend(); it++) {
        for (const auto &flow : it->second) {
            prepareToExport(flow);
        }
    }
}

bool FlowCache::exportCacheFull() { return exportCache.size() >= MAX_PACKETS ? true : false; }

std::string FlowCache::getFlowKey(const Flow &flow) {
    return std::to_string(flow.srcIP) + std::to_string(flow.srcPort) + std::to_string(flow.destIP) +
           std::to_string(flow.destPort);
}

std::queue<struct NetflowRecord> &FlowCache::getExportCache() { return exportCache; }

