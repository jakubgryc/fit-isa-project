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
#include "UDPExporter.h"
#include "Tools.h"

struct PcapData {
    uint32_t srcIP;
    uint32_t destIP;
    uint16_t srcPort;
    uint16_t destPort;
    struct timeval timeData;
    uint8_t tcpFlags;
};

/**
 * @class PcapHandler
 * @brief PcapHandler class
 *
 */
class PcapHandler {
   public:
    /**
     * @brief Construct a new Pcap Handler object
     *
     * @param pcapFile path to pcap file
     */
    PcapHandler(std::string &pcapFile);

    /**
     * @brief Destroy the Pcap Handler object
     *
     */
    ~PcapHandler();

    /**
     * @brief Open pcap file
     *
     * @return true if file was opened
     * @return false if file was not opened
     */
    bool openPcap();

    /**
     * @brief Start the pcap handler
     *
     * @param connection exporter
     * @param timer timer object for time handling
     */
    void start(UDPExporter *connection, Timer &timer);

   private:
    /**
     * @brief Proccess packet, extract important data from packet
     *
     * @param header pcap header
     * @param packet packet data
     * @param pData pcap data to be filled
     * @return int payload size
     */
    int proccessPacket(const struct pcap_pkthdr *header, const u_char *packet, PcapData *pData);

    std::string filePath;
    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];
};

#endif
