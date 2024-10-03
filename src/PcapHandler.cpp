/**
 * @file PcapHandler.cpp
 * @brief PcapHandler class implementation
 * @author Jakub Gryc <xgrycj03>
 */

#include "../include/PcapHandler.h"

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
    std::cout << "(" << packet_id++ << ") inside Packet processing\n";
    return;
}
