
#ifndef TOOLS_H
#define TOOLS_H

#include <sys/time.h>

#include <cstdint>
#include <string>

struct Arguments {
    std::string hostname;
    int port;
    std::string pcap_file;
    int active_timeout = 60;
    int inactive_timeout = 60;
};

/**
 * @class Timer
 * @brief Timer interface to handle time functionality>
 *
 */
class Timer {
   public:
    /**
     * @brief Timer constructor
     *
     * @param Active timeout active timeout parsed from arguments (or implicitly 60)
     * @param inactiveTimeout Inactive timeout parsed from arguments (or implicitly 60)
     */
    Timer(int activeTimeout, int inactiveTimeout);

    /**
     * @brief Function returns current SysUptime.
     * @note Due to the nature of the project the uptime will be
     *        very small number approaching zero
     * @return System uptime since the program has started in milliseconds
     */
    uint32_t getSysUptime();

    /**
     * @brief Returns a tuple with unix_secs and unix_nsecs to be added into Netflow heade
     */
    std::tuple<uint32_t, uint32_t, uint32_t> getEpochTuple();

    /**
     * @brief Calculates the difference of times in milliseconds
     * @brief t1 - t2
     *
     * @param t1 timeval structure of more recent timestamp
     * @param t2 timeval structure of older timestamp
     * @return Time difference in milliseconds
     */
    uint32_t getTimeDifference(struct timeval *t1, struct timeval *t2);

    struct timeval *getStartTime();

    /**
     * @brief Function to determine if the flow has extended either the active or inactive timeour
     *
     * @param firstSeenTime 
     * @param lastSeenTime 
     * @param currentTime 
     * @return true if either one of the timers are expired, resulting in sending the flow, in other case false
     */
    bool checkFlowTimeouts(uint32_t firstSeenTime, uint32_t lastSeenTime, uint32_t currentTime, uint32_t *expirationTime);

    void printStartTime();

   private:
    struct timeval programStartTime;
    uint32_t activeTimeout;
    uint32_t inactiveTimeout;
};

/**
 * @brief Error printing function
 */
void print_err();

/**
 * @brief Helper function to correctly parse program arguments
 *
 * @param argc
 * @param argv
 * @param args Argument structure to hold the argument information
 * @return
 */
bool parse_arguments(int argc, char *argv[], Arguments *args);

#endif
