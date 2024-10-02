
#include "../include/UDPConnection.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

UDPConnection::UDPConnection(const std::string hostname, int port) : hostname(hostname), port(port), sockfd(-1) {
    memset(&server_address, 0, sizeof(server_address));
}

UDPConnection::~UDPConnection() {
    if (sockfd != -1) {
        close(sockfd);
        std::cout << "Closed?\n";
    }
}

bool UDPConnection::resolveHostname() {
    struct addrinfo hints, *result;
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    status = getaddrinfo(hostname.c_str(), nullptr, &hints, &result);
    if (status != 0) {
        std::cerr << "Error: Failure while resolving hostname\n";
        return false;
    }

    memcpy(&(server_address), result->ai_addr, result->ai_addrlen);
    freeaddrinfo(result);
    return true;
}

bool UDPConnection::connect() {
    if (!resolveHostname()) {
        return false;
    }

    server_address.sin_port = htons(port);
    server_address.sin_family = AF_INET;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        std::cerr << "Error: Could not create socket\n";
        return false;
    }

    return true;
}

bool UDPConnection::send_flow(const char *data) {
    ssize_t bytes_tx =
        sendto(sockfd, data, strlen(data), 0, (struct sockaddr *)(&server_address), sizeof(server_address));
    std::cout << "Sending: " << data;
    if (bytes_tx < 0) {
        std::cerr << "error: failed to send data\n";
    }
    return true;
}

void UDPConnection::printData() {
    std::cout << "hostname is: |" << hostname << "|\n";
    std::cout << "port is: |" << port << "|\n";
}
