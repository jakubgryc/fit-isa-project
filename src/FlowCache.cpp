

#include "../include/FlowCache.h"

#include <netinet/in.h>

void FlowCache::updateFlow(const Flow &flow, uint32_t packetSize) {
    std::string flowKey = getFlowKey(flow);

    auto it = flowCache.find(flowKey);

    if (it == flowCache.end()) {
        // Flow not in flowcache, create a new one
        // std::cout << "Flow doesnt exist, CREATING NEW ONE\n";
        flowCache[flowKey] = std::make_shared<Flow>(flow);
        flowCache[flowKey]->update(packetSize, std::time(nullptr));
    } else {
        // Flow is already in flowcache, update its information
        // std::cout << "Flow exists, UPDATING\n";
        it->second->update(packetSize, flow.lastSeenTime);
    }

    return;
}

std::string FlowCache::getFlowKey(const Flow &flow) {
    return std::to_string(flow.srcIP) + std::to_string(flow.srcPort) + std::to_string(flow.destIP) +
           std::to_string(flow.destPort);
}

void FlowCache::print() {
    int cnt = 1;

    uint32_t totalBytes = 0;
    uint32_t totalPackets = 0;

    std::cout << "\n-----------------FLOW DATA------------------\n";
    std::cout << "             SRCIP:SRCPORT -> DSTIP:DSTPORT    BYTES\n" ;
    for (auto it = flowCache.begin(); it != flowCache.end(); it++) {
        struct in_addr ip_addrSRC;
        struct in_addr ip_addrDEST;

        uint16_t srcPort = ntohs(it->second->srcPort);
        uint16_t destPort = ntohs(it->second->destPort);

        ip_addrSRC.s_addr = it->second->srcIP;
        ip_addrDEST.s_addr = it->second->destIP;

        char *srcIP = inet_ntoa(ip_addrSRC);
        char *destIP = inet_ntoa(ip_addrDEST);

        uint32_t byteCount = it->second->byteCount;
        uint32_t packetCount = it->second->packetCount;
        std::cout << "Flow no: " << cnt << ":  ";
        std::cout << srcIP << ":" << srcPort << " -> " << destIP << ":" << destPort << "    " << byteCount << std::endl;
        cnt++;
        totalBytes += byteCount;
        totalPackets += packetCount;
    }

    std::cout << "Total packets: " << totalPackets << std::endl;
    std::cout << "Total bytes:   " << totalBytes << std::endl;
}
