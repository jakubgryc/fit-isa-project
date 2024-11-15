/**
 * @file Tools.cpp
 * @brief Helper tools containing argument parsing and handling time
 * @author Jakub Gryc <xgrycj03>
 */

#include "../include/Tools.h"

#include <iostream>

Timer::Timer(int activeTimeout, int inactiveTimeout)
    : activeTimeout(static_cast<uint32_t>(activeTimeout)), inactiveTimeout(static_cast<uint8_t>(inactiveTimeout)) {
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

std::tuple<uint32_t, uint32_t, uint32_t> Timer::getEpochTuple() {
    struct timeval currentTime;
    std::tuple<uint32_t, uint32_t, uint32_t> resTuple;

    gettimeofday(&currentTime, nullptr);

    resTuple = std::make_tuple(getTimeDifference(&currentTime, &programStartTime), currentTime.tv_sec,
                               currentTime.tv_usec * 1000);
    return resTuple;
}

uint32_t Timer::getTimeDifference(struct timeval *t1, struct timeval *t2) {
    // Note:
    // If the time difference is more than 49 days (UINT32 MAX in milliseconds is roughly 49 days), there may be a
    // uint32 overflow In that case it may not display the time properly
    //
    // When analysing the pcap timestamp, the time will be correct if only interpreted as signed integer, because
    // the packet was captured in the past when compared to the current time.
    // uint32_t timeDiff_m;
    // int32_t seconds = t1->tv_sec - t2->tv_sec;
    // int32_t microseconds = t1->tv_usec - t2->tv_usec;
    //
    // if (microseconds < 0) {
    //     microseconds += 1000000L;
    //     seconds--;
    // }
    //
    // timeDiff_m = seconds * 1000 + microseconds / 1000;
    // return timeDiff_m;

    struct timeval res;

    res.tv_sec = t1->tv_sec - t2->tv_sec;
    res.tv_usec = t1->tv_usec - t2->tv_usec;
    if (res.tv_usec < 0) {
        res.tv_usec += 1000000L;
        res.tv_sec--;
    }
    return ((uint32_t)res.tv_sec * 1000 + (uint32_t)res.tv_usec / 1000);
}

bool Timer::checkFlowTimeouts(struct timeval firstSeenTime, struct timeval lastSeenTime, struct timeval currentTime,
                              uint32_t *expirationTime) {
    // The arguments are in milliseconds, the active and inactive timeouts are in seconds,
    // so we need to convert the timeouts to milliseconds
    // int64_t first = static_cast<int64_t>(firstSeenTime);
    // int64_t last = static_cast<int64_t>(lastSeenTime);
    // int64_t current = static_cast<int64_t>(currentTime);
    // int64_t activeT = static_cast<int64_t>(activeTimeout);
    // int64_t inactiveT = static_cast<int64_t>(inactiveTimeout);
    int32_t expTimeActive = 0;
    int32_t expTimeInactive = 0;
    bool expired = false;

    // Correctly handle the expiration times
    if ((currentTime.tv_sec - firstSeenTime.tv_sec > activeTimeout) ||
        ((currentTime.tv_sec - firstSeenTime.tv_sec == activeTimeout) &&
         (currentTime.tv_usec - firstSeenTime.tv_usec > 0L))) {
        expired = true;
        expTimeActive = getTimeDifference(&currentTime, &firstSeenTime);
    }

    if ((currentTime.tv_sec - lastSeenTime.tv_sec > inactiveTimeout) ||
        ((currentTime.tv_sec - lastSeenTime.tv_sec == inactiveTimeout) &&
         (currentTime.tv_usec - lastSeenTime.tv_usec > 0L))) {
        expired = true;
        expTimeInactive = getTimeDifference(&currentTime, &lastSeenTime);
    }

    // Handle the expiration time, if both active and inactive timeouts are expired, return the larger one
    *expirationTime =
        expTimeActive > expTimeInactive ? expTimeActive : expTimeInactive;

    return expired;
}

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
