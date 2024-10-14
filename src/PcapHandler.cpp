/**
 * @file PcapHandler.cpp
 * @brief PcapHandler class implementation
 * @author Jakub Gryc <xgrycj03>
 */

#include "../include/PcapHandler.h"

#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <cstring>

#include "../include/Flow.h"

/**
 * @brief PcapHandler constructor
 *
 * @param pcapFile PCAP source file
 */
PcapHandler::PcapHandler(std::string &pcapFile) : filePath(pcapFile), handle(nullptr) {}

/**
 * @brief PcapHandler destroyer
 */
PcapHandler::~PcapHandler() {
    if (handle) {
        pcap_close(handle);
    }
}

bool PcapHandler::openPcap() {
    handle = pcap_open_offline(filePath.c_str(), errbuf);

    if (handle == nullptr) {
        std::cerr << "Error opening pcap_file: " << errbuf << std::endl;
        return false;
    }
    return true;
}

void PcapHandler::start(UDPExporter *exporter, Timer &timer) {
    if (handle == nullptr) {
        // Should not happen
        std::cerr << "Error: Pcap file is not opened\n";
        return;
    }

    FlowCache flowCache(timer);
    PcapData pcapData;

    exporter->printData();

    const u_char *packet;
    struct pcap_pkthdr header;

    int payloadSize = 0;

    while ((packet = pcap_next(handle, &header)) != nullptr) {
        memset(&pcapData, 0, sizeof(struct PcapData));
        payloadSize = proccessPacket(&header, packet, &pcapData);

        if (payloadSize != -1) {
            if (flowCache.exportCacheFull()) {
                // export to collector
                exporter->sendFlows(flowCache.getExportCache(), timer.getEpochTuple(), true);
            }

            Flow flow(pcapData.srcIP, pcapData.destIP, pcapData.srcPort, pcapData.destPort, pcapData.tcpFlags);

            flowCache.handleFlow(flow, static_cast<uint32_t>(payloadSize), pcapData.timeData);
        }
    }

    flowCache.flushToExportAll();
    std::cout << "Got here???\n";
    exporter->sendFlows(flowCache.getExportCache(), timer.getEpochTuple(), false);
    flowCache.print();
}

int PcapHandler::proccessPacket(const struct pcap_pkthdr *header, const u_char *packet, PcapData *pData) {
    int payloadSize = -1;

    const struct ether_header *eth_header = (struct ether_header *)packet;

    if (ntohs(eth_header->ether_type) == ETHERTYPE_IP) {
        const struct ip *ip_header = (struct ip *)(packet + sizeof(struct ether_header));

        unsigned int ip_len = ip_header->ip_hl * 4;

        struct in_addr srcIPStruct = ip_header->ip_src;
        struct in_addr destIPStruct = ip_header->ip_dst;
        if (ip_header->ip_p == IPPROTO_TCP) {
            const struct tcphdr *tcp_header = (struct tcphdr *)(packet + sizeof(struct ether_header) + ip_len);
            struct timeval tv = header->ts;

            uint32_t srcIP = srcIPStruct.s_addr;
            uint32_t destIP = destIPStruct.s_addr;

            uint16_t srcPort = tcp_header->source;
            uint16_t destPort = tcp_header->dest;
            uint8_t tcpFlags = tcp_header->th_flags;

            payloadSize = header->len - sizeof(struct ether_header);

            pData->srcIP = srcIP;
            pData->destIP = destIP;
            pData->srcPort = srcPort;
            pData->destPort = destPort;
            pData->timeData = tv;
            pData->tcpFlags = tcpFlags;
        }
    }
    return payloadSize;
}
