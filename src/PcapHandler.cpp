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

void PcapHandler::start() {
    if (handle == nullptr) {
        // Should not happen
        std::cerr << "Error: Pcap file is not opened\n";
        return;
    }

    const u_char *packet;
    struct pcap_pkthdr header;

    while ((packet = pcap_next(handle, &header)) != nullptr) {
        proccessPacket(&header, packet);
    }
}

void PcapHandler::proccessPacket(const struct pcap_pkthdr *header, const u_char *packet) {
    static int packet_id = 1;

    const struct ether_header *eth_header = (struct ether_header *)packet;

    if (ntohs(eth_header->ether_type) == ETHERTYPE_IP) {
        const struct ip *ip_header = (struct ip *)(packet + sizeof(struct ether_header));

        unsigned int ip_len = ip_header->ip_hl;

        char srcIP[INET_ADDRSTRLEN];
        char destIP[INET_ADDRSTRLEN];
        if (ip_header->ip_p == IPPROTO_TCP) {
            memset(srcIP, 0, INET_ADDRSTRLEN);
            memset(destIP, 0, INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &(ip_header->ip_src), srcIP, INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &(ip_header->ip_dst), destIP, INET_ADDRSTRLEN);

            const struct tcphdr *tcp_header = (struct tcphdr *)(packet + sizeof(struct ether_header) + ip_len * 4);

            uint16_t srcPort = ntohs(tcp_header->source);
            uint16_t destPort = ntohs(tcp_header->dest);

            std::cout << "Packet no.: " << packet_id << std::endl;
            std::cout << srcIP << ":" << srcPort << " --->  ";
            std::cout << destIP << ":" << destPort << std::endl;
            std::cout << "\n";
            packet_id++;
        }
    }
    return;
}
