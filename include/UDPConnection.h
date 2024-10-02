/**
 * @file UDPConnection.h
 * @brief UDP Connection header file
 * @author Jakub Gryc <xgrycj03>
 */

#ifndef UDPCONNECTION_H
#define UDPCONNECTION_H

#include <netinet/in.h>

#include <string>

class UDPConnection {
   public:
    UDPConnection(const std::string hostname, int port);
    ~UDPConnection();

    bool connect();
    bool send_flow(const char *data);
    void printData();

   private:
    bool resolveHostname();


    struct sockaddr_in server_address;

    const std::string hostname;
    int port;
    int sockfd;
};

#endif
