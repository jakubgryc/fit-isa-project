

#include "../include/FlowCache.h"

#include <netinet/in.h>

#include <bitset>

FlowCache::FlowCache(Timer &timer) : timer(timer) {}

void FlowCache::handleFlow(const Flow &flow, uint32_t packetSize, struct timeval packetTime) {
    std::string flowKey = getFlowKey(flow);

    auto it = flowCache.find(flowKey);

    uint32_t packetTimestamp = timer.getTimeDifference(&packetTime, timer.getStartTime());

    if (it == flowCache.end()) {
        // Flow not in flowcache, create a new one
        // std::cout << "Flow doesnt exist, CREATING NEW ONE\n";
        flowCache[flowKey] = std::make_shared<Flow>(flow);
        flowCache[flowKey]->setFirst(packetTimestamp, flow.tcpFlags);
        flowCache[flowKey]->update(packetSize, packetTimestamp);
    } else {
        // Flow is already in flowcache, update its information
        // std::cout << "Flow exists, UPDATING\n";
        if (timer.checkFlowTimeouts(it->second->startTime, it->second->lastSeenTime, packetTimestamp)) {
            std::cout << "TIMEOUT OVEREACHED!!!!!!!\n";
            // timeout has overeach, prepare to export into export cache
            prepareToExport(it->second);
            it->second->setFirst(packetTimestamp, flow.tcpFlags);
        }
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

bool FlowCache::exportCacheFull() { return exportCache.size() == 30 ? true : false; }

std::string FlowCache::getFlowKey(const Flow &flow) {
    return std::to_string(flow.srcIP) + std::to_string(flow.srcPort) + std::to_string(flow.destIP) +
           std::to_string(flow.destPort);
}

std::queue<struct NetflowRecord> &FlowCache::getExportCache() { return exportCache; }

void FlowCache::print() {
    int cnt = 1;

    uint32_t totalBytes = 0;
    uint32_t totalPackets = 0;

    std::cout << "\n-----------------FLOW DATA------------------\n";
    std::cout << "             SRCIP:SRCPORT -> DSTIP:DSTPORT    BYTES\n";
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
        std::cout << srcIP << ":" << srcPort << " -> " << destIP << ":" << destPort << "  "
                  << std::bitset<8>(it->second->tcpFlags) << "      " << byteCount << std::endl;
        std::cout << "First: " << it->second->startTime << std::endl;
        std::cout << "Last : " << it->second->lastSeenTime << std::endl;
        std::cout << "\n";
        cnt++;
        totalBytes += byteCount;
        totalPackets += packetCount;
    }

    std::cout << "Total packets: " << totalPackets << std::endl;
    std::cout << "Total bytes:   " << totalBytes << std::endl;
}
