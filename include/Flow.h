/**
 * @file Flow.h
 * @brief Flow header
 * @author Jakub Gryc <xgrycj03>
 */

#ifndef FLOW_H
#define FLOW_H

#include <cstdint>
#include <ctime>
#include <string>

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
    std::string srcIP, destIP;
    uint16_t srcPort, destPort;
    uint8_t protocol;
    uint64_t packetCount, byteCount;
    std::time_t startTime, lastSeenTime;

    /**
     * @brief Constructor for a Flow class
     *
     * @param srcIP Source IP address 
     * @param destIP Destination IP address
     * @param srcPort Source port number
     * @param destPort Destination port number
     * @param protocol Protocol
     */
    Flow(std::string srcIP, std::string destIP, uint16_t srcPort, uint16_t destPort, uint8_t protocol);

    /**
     * @brief Function to update the flow statistics
     *
     * @param packetSize Packet size to be added to the flow
     * @param timestamp Current timestamp of the captured packet
     */
    void update(uint64_t packetSize, std::time_t timestamp);

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
