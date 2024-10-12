/**
 * @file Flow.cpp
 * @brief Flow class implementation
 * @author Jakub Gryc <xgrycj03>
 */

#include "../include/Flow.h"

Flow::Flow(uint32_t srcIP, uint32_t destIP, uint16_t srcPort, uint16_t destPort, uint8_t tcpFlags)
    : srcIP(srcIP),
      destIP(destIP),
      srcPort(srcPort),
      destPort(destPort),
      tcpFlags(tcpFlags),
      packetCount(0),
      byteCount(0),
      startTime(0),
      lastSeenTime(0) {
}

void Flow::setFirst(uint32_t packetTime) {
    startTime = packetTime;
}

void Flow::update(uint32_t packetSize, uint32_t timestamp) {
    packetCount++;
    byteCount += packetSize;
    lastSeenTime = timestamp;
    return;
}
