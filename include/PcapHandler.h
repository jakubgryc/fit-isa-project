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
#include <pcap/pcap.h>
#include <pcap.h>

#include <iostream>
#include <string>

class PcapHandler {
   public:
    PcapHandler(std::string &pcapFile);

    ~PcapHandler();

    bool openPcap();

    void start();

   private:
    void proccessPacket(const struct pcap_pkthdr *header, const u_char *packet);

    std::string filePath;
    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];
};

#endif
