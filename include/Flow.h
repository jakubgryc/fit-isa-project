/**
 * @file Flow.h
 * @brief Flow header
 * @author Jakub Gryc <xgrycj03>
 */

#ifndef FLOW_H
#define FLOW_H

#include <cstdint>
#include <ctime>
#include <iostream>
#include <string>

/**
 * @class NetflowHeader
 * @brief Structure to store Netflow v5 Header information
 *
 */
struct NetflowHeader {
    uint16_t version;
    uint16_t flowCount;
    uint32_t sysUptime;
    uint32_t unix_secs;
    uint32_t unix_nsecs;
    uint32_t flowSequence;
    uint8_t engine_type;
    uint8_t engine_id;
    uint16_t sampling_interval;
};

/**
 * @class NetflowRecord
 * @brief Structure to store Netflow v5 record information
 *
 */
struct NetflowRecord {
    uint32_t srcIP;
    uint32_t destIP;
    uint32_t nexthop;
    uint16_t SNMPinput;
    uint16_t SNMPoutput;
    uint32_t totalPackets;
    uint32_t totalBytes;
    uint32_t firstSeen;
    uint32_t lastSeen;
    uint16_t srcPort;
    uint16_t destPort;
    uint8_t pad1;
    uint8_t TCPflags;
    uint8_t protocol;
    uint8_t tos;
    uint16_t srcAS;
    uint16_t destAS;
    uint8_t srcMask;
    uint8_t destMask;
    uint16_t pad2;
};

/**
 * @class Flow
 * @brief Class representing single TCP Flow
 *
 * The class implementation stores and updates the state of each individual
 * network flow. Stores source and destination IP addresses, ports and byte/packet counts.
 * The flow gets updated when the packets to the belonging flow gets captured.
 */
class Flow {
   public:
    uint32_t srcIP, destIP;
    uint16_t srcPort, destPort;
    uint32_t packetCount, byteCount;
    uint32_t startTime, lastSeenTime;

    /**
     * @brief Constructor for a Flow class
     *
     * @param srcIP Source IP address
     * @param destIP Destination IP address
     * @param srcPort Source port number
     * @param destPort Destination port number
     * @param protocol Protocol
     */
    Flow(uint32_t srcIP, uint32_t destIP, uint16_t srcPort, uint16_t destPort);

    /**
     * @brief Function sets the first seen timestamp in milliseconds
     *
     * @param packetTime 
     */
    void setFirst(uint32_t packetTime);

    /**
     * @brief Function to update the flow statistics
     *
     * @param packetSize Packet size to be added to the flow
     * @param timestamp Current timestamp of the captured packet
     */
    void update(uint32_t packetSize, uint32_t timestamp);

    /**
     * @brief Function to determine if the flow is inactive
     *
     * @param currentTime Current time
     * @param inactiveTimeout Inactive timeout specified by user (or implicit 30 seconds) [in seconds]
     * @return True if the flow has been inactive for longer than the timeout
     */
    bool isInactive(std::time_t currentTime, std::time_t inactiveTimeout);
};

#endif  // !_FLOW_H
