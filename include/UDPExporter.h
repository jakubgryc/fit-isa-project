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
#include "Tools.h"

/**
 * @class UDPExporter
 * @brief Interface to manage exporting individual flows via UDP
 *
 */
class UDPExporter {
   public:
    /**
     * @brief Constructor of UDPExporter class
     *
     * @param hostname hostname or IP address of a collector
     * @param port UDP port of a collector
     */
    UDPExporter(const std::string hostname, int port);

    /**
     * @brief Destroyer
     */
    ~UDPExporter();

    bool connect();
    bool sendFlows(std::queue<struct NetflowRecord> &exportCache, Timer &timer,
                   bool sendOnlyMAX);
    void printData();

   private:
    bool resolveHostname();

    struct sockaddr_in server_address;

    const std::string hostname;
    int port;
    int sockfd;
};

#endif
