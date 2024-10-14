/**
 * @file UDPExporter.h
 * @brief UDP Exporter header file
 * @author Jakub Gryc <xgrycj03>
 */

#ifndef UDPCONNECTION_H
#define UDPCONNECTION_H

#include <netinet/in.h>

#include <queue>
#include <string>

#include "Flow.h"

class UDPExporter {
   public:
    UDPExporter(const std::string hostname, int port);
    ~UDPExporter();

    bool connect();
    bool sendFlows(std::queue<struct NetflowRecord> &exportCache, std::tuple<uint32_t, uint32_t, uint32_t> epochTuple, bool sendOnlyMAX);
    void printData();

   private:
    bool resolveHostname();

    struct sockaddr_in server_address;

    const std::string hostname;
    int port;
    int sockfd;
};

#endif
