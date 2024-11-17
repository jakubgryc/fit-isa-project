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
     * @brief Destroyer of the UDP Exporter
     * correctly handles closing of a socket
     */
    ~UDPExporter();

    /**
     * @brief Function to create a socket and tries to connect to a remote port
     *
     * @return true if no problem with connecting, else false
     */
    bool connect();

    /**
     * @brief Function which handles exporting already expirated flows
     *
     * @param exportCache queue of Netflow Records
     * @param timer Timer class instance
     * @param sendOnlyMAX if set to true the exporter maximizes the number of flows to send up to 30, 
     *        if there are less then 30 flows then it will not send any data
     */
    bool sendFlows(std::queue<struct NetflowRecord> &exportCache, Timer &timer,
                   bool sendOnlyMAX);

   private:
    /**
     * @brief Function which resolves hostname to IPv4 address
     *
     * @return true if successed
     */
    bool resolveHostname();

    struct sockaddr_in server_address;

    const std::string hostname;
    int port;
    int sockfd;
};

#endif
