/**
 * @file Flow.cpp
 * @brief Flow class implementation
 * @author Jakub Gryc <xgrycj03>
 */

#include "../include/Flow.h"

Flow::Flow(uint32_t srcIP, uint32_t destIP, uint16_t srcPort, uint16_t destPort, uint8_t protocol)
    : srcIP(srcIP),
      destIP(destIP),
      srcPort(srcPort),
      destPort(destPort),
      protocol(protocol),
      packetCount(0),
      byteCount(0),
      startTime(std::time(nullptr)),
      lastSeenTime(startTime) {}
