/**
 * @file tools.cpp
 * @brief Helper tools containing argument parsing and handling time
 */

#include "../include/Tools.h"

#include <iostream>

Timer::Timer(int activeTimeout, int inactiveTimeout) : activeTimeout(activeTimeout), inactiveTimeout(inactiveTimeout) {
    gettimeofday(&programStartTime, nullptr);
}

uint32_t Timer::getSysUptime() {
    struct timeval currentTime;
    gettimeofday(&currentTime, nullptr);

    uint32_t sysUptime = getTimeDifference(&currentTime, &programStartTime);

    // Note to myself:
    // If the program runs for more than 49 days, there may be uint32 overflow. (UINT32MAX in milliseconds is roughly 49
    // days) Due to the nature of the project, the chance of this happening is almost zero.
    return sysUptime;
}

uint32_t Timer::getTimeDifference(struct timeval *t1, struct timeval *t2) {
    // Note:
    // If the time difference is more than 49 days (UINT32 MAX in milliseconds is roughly 49 days), there may be a
    // uint32 overflow In that case it may not display the time properly
    //
    // When analysing the pcap timestamp, the time will be correct if only interpreted as signed integer, because
    // the packet was captured in the past when compared to the run of this program.
    uint32_t timeDiff_m;
    int32_t seconds = t1->tv_sec - t2->tv_sec;
    int32_t microseconds = t1->tv_usec - t2->tv_usec;

    if (microseconds < 0) {
        microseconds += 1000000L;
        seconds--;
    }

    timeDiff_m = seconds * 1000 + microseconds / 1000;
    return timeDiff_m;
}

void Timer::printStartTime() { std::cout << "start time is: " << programStartTime.tv_sec << std::endl; }

struct timeval *Timer::getStartTime() { return &programStartTime; }

void print_err() {
    std::cerr << "Usage: ./p2nprobe <host>:<port> <pcap_file_path> [-a <active_timeout> -i <inactive_timeout>]\n";
}

bool parse_arguments(int argc, char *argv[], Arguments *args) {
    //
    // Check if the mandatory arguments <host>:<port> and <pcap_file_path> are provided
    if (argc < 3) {
        return false;
    }

    bool parsed_hostname = false;
    bool parsed_pcap_file = false;
    int timeout = 60;
    std::string current_arg;

    for (int i = 1; i < argc; i++) {
        current_arg = argv[i];

        size_t colonPos = current_arg.find(':');
        if (colonPos != std::string::npos) {
            args->hostname = current_arg.substr(0, colonPos);
            args->port = stoi(current_arg.substr(colonPos + 1));
            parsed_hostname = true;

        } else if (current_arg == "-a" || current_arg == "-i") {
            if (argv[++i] != NULL) {
                try {
                    timeout = std::stoi(argv[i]);
                } catch (std::invalid_argument const &ex) {
                    // Catch error if the given timeout is not a number
                    std::cerr << "No timeout given\n";
                    return false;
                }
                current_arg == "-a" ? args->active_timeout = timeout : args->inactive_timeout = timeout;
            } else {
                return false;
            }
        } else {
            args->pcap_file = current_arg;
            parsed_pcap_file = true;
        }
    }

    if (!parsed_hostname || !parsed_pcap_file) return false;

    return true;
}
