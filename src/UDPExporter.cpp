
#include "../include/UDPExporter.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

UDPExporter::UDPExporter(const std::string hostname, int port) : hostname(hostname), port(port), sockfd(-1) {
    memset(&server_address, 0, sizeof(server_address));
}

UDPExporter::~UDPExporter() {
    if (sockfd != -1) {
        close(sockfd);
    }
}

bool UDPExporter::resolveHostname() {
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

bool UDPExporter::connect() {
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

bool UDPExporter::sendFlows(std::queue<NetflowRecord> &exportCache, Timer &timer,
                            bool sendOnlyMAX) {
    static uint32_t flowSequence = 0;

    while (!exportCache.empty() && (!sendOnlyMAX || exportCache.size() >= MAX_PACKETS)) {
        size_t totalFlows = exportCache.size();
        if (totalFlows > MAX_PACKETS) totalFlows = MAX_PACKETS;

        std::tuple<uint32_t, uint32_t, uint32_t> epochTuple = timer.getEpochTuple();

        struct NetflowHeader header;
        header.version = htons(5);
        header.flowCount = htons(static_cast<uint16_t>(totalFlows));
        header.sysUptime = htonl(std::get<0>(epochTuple));
        header.unix_secs = htonl(std::get<1>(epochTuple));
        header.unix_nsecs = htonl(std::get<2>(epochTuple));
        header.flowSequence = htonl(flowSequence);
        header.engine_type = 0;
        header.engine_id = 0;
        header.sampling_interval = htons(0);

        // calculate the totalSize and clamp it to 30 packets

        size_t totalSize = sizeof(struct NetflowHeader) + sizeof(struct NetflowRecord) * totalFlows;

        char *buffer = new char[totalSize];
        memcpy(buffer, &header, sizeof(struct NetflowHeader));
        size_t currentOffset = sizeof(struct NetflowHeader);
        for (size_t i = 0; i < totalFlows; i++) {
            struct NetflowRecord &nflwRd = exportCache.front();
            memcpy(buffer + currentOffset, &nflwRd, sizeof(struct NetflowRecord));
            currentOffset += sizeof(struct NetflowRecord);
            exportCache.pop();
        }

        ssize_t bytes_tx =
            sendto(sockfd, buffer, totalSize, 0, (struct sockaddr *)(&server_address), sizeof(server_address));

        if (bytes_tx < 0) {
        }

        flowSequence += totalFlows;

        delete[] buffer;
    }

    return true;
}

