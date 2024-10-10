
#ifndef TOOLS_H
#define TOOLS_H

#include <chrono>
#include <string>

struct Arguments {
    std::string hostname;
    int port;
    std::string pcap_file;
    int active_timeout = 60;
    int inactive_timeout = 60;
};

class Timer {
   public:
    Timer(int activeTimeout, int inactiveTimeout);
    uint32_t getSysUptime() const;

   private:
    std::chrono::high_resolution_clock::time_point programStartTime;
    int activeTimeout;
    int inactiveTimeout;
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
