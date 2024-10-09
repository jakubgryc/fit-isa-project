/**
 * @file PcapHandler.cpp
 * @brief PcapHandler class implementation
 * @author Jakub Gryc <xgrycj03>
 */

#include "../include/PcapHandler.h"
#include "../include/Flow.h"

#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <cstring>

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

void PcapHandler::start(UDPConnection *connection) {
    if (handle == nullptr) {
        // Should not happen
        std::cerr << "Error: Pcap file is not opened\n";
        return;
    }

    FlowCache flowCache;
    std::cout << "size of flowCache is: " << sizeof(flowCache) << std::endl;

    const u_char *packet;
    struct pcap_pkthdr header;

    int count;
    while ((packet = pcap_next(handle, &header)) != nullptr) {
        count = proccessPacket(&header, packet, flowCache);
    }



    flowCache.print();

    std::cout << "Packet count is: " << count << std::endl;
    std::cout << "size of flowCache is: " << sizeof(flowCache) << std::endl;
}

int PcapHandler::proccessPacket(const struct pcap_pkthdr *header, const u_char *packet, FlowCache &flowCache) {
    static int packet_id = 0;

    const struct ether_header *eth_header = (struct ether_header *)packet;

    if (ntohs(eth_header->ether_type) == ETHERTYPE_IP) {
        const struct ip *ip_header = (struct ip *)(packet + sizeof(struct ether_header));

        unsigned int ip_len = ip_header->ip_hl * 4;
        uint32_t byteCount = 0;

        // char srcIP[INET_ADDRSTRLEN];
        // char destIP[INET_ADDRSTRLEN];

        struct in_addr srcIPStruct = ip_header->ip_src;
        struct in_addr destIPStruct = ip_header->ip_dst;
        if (ip_header->ip_p == IPPROTO_TCP) {
            const struct tcphdr *tcp_header = (struct tcphdr *)(packet + sizeof(struct ether_header) + ip_len);

            uint32_t tcp_len = sizeof(*tcp_header);
            byteCount = header->len - sizeof(struct ether_header);
            // memset(srcIP, 0, INET_ADDRSTRLEN);
            //
            // memset(destIP, 0, INET_ADDRSTRLEN);
            // inet_ntop(AF_INET, &(ip_header->ip_src), srcIP, INET_ADDRSTRLEN);
            // inet_ntop(AF_INET, &(ip_header->ip_dst), destIP, INET_ADDRSTRLEN);

            // uint16_t srcPort = ntohs(tcp_header->source);
            // uint16_t destPort = ntohs(tcp_header->dest);

            // std::cout << "Packet no.: " << packet_id << std::endl;
            // std::cout << srcIP << ":" << srcPort << " --->  ";
            // std::cout << destIP << ":" << destPort << std::endl;
            // std::cout << "\n";
            packet_id++;

            uint32_t srcIP = srcIPStruct.s_addr;
            uint32_t destIP = destIPStruct.s_addr;

            uint16_t srcPort = tcp_header->source;
            uint16_t destPort = tcp_header->dest;

            Flow flow(srcIP, destIP, srcPort, destPort, IPPROTO_TCP);

            flowCache.updateFlow(flow, byteCount);
        }
    }

    return packet_id;
}
