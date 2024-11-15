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
      startTime(),
      lastSeenTime() {
}

void Flow::setFirst(struct timeval packetTime, uint8_t tcpflgs) {
    this->clear();
    startTime = packetTime;
    tcpFlags = tcpflgs;
}

void Flow::update(uint32_t packetSize, struct timeval timestamp) {
    packetCount++;
    byteCount += packetSize;
    lastSeenTime = timestamp;
}

void Flow::clear() {
    packetCount = 0;
    byteCount = 0;
    startTime.tv_sec = 0;
    startTime.tv_usec = 0;
    lastSeenTime.tv_sec = 0;
    lastSeenTime.tv_usec = 0;
    tcpFlags = 0;
}
