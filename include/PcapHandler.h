/**
 * @file PcapHandler.h
 * @brief PcapHandler header file
 * @author Jakub Gryc <xgrycj03>
 */

#ifndef PCAP_HANDLER_H
#define PCAP_HANDLER_H

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <pcap.h>
#include <pcap/pcap.h>

#include <iostream>
#include <string>

#include "FlowCache.h"
#include "UDPConnection.h"
#include "Tools.h"

struct PcapData {
    uint32_t srcIP;
    uint32_t destIP;
    uint16_t srcPort;
    uint16_t destPort;
    struct timeval timeData;
};

/**
 * @class PcapHandler
 * @brief PcapHandler class
 *
 */
class PcapHandler {
   public:
    PcapHandler(std::string &pcapFile);

    ~PcapHandler();

    bool openPcap();

    void start(UDPConnection *connection, Timer &timer);

   private:
    int proccessPacket(const struct pcap_pkthdr *header, const u_char *packet, PcapData *pData);

    std::string filePath;
    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];
};

#endif
