/**
 * @file tools.cpp
 * @brief Helper tools containing argument parsing and handling time
 */

#include <iostream>

#include "../include/tools.h"


Timer::Timer() : programStartTime(std::chrono::high_resolution_clock::now()) {}


uint32_t Timer::timeDifference() const {
    auto currentTime = std::chrono::high_resolution_clock::now();

    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - programStartTime).count();


    // Note to myself:
    // If the program runs for more than 49 days, there may be uint32 overflow.
    // Due to the nature of the project, the chance of this happening is almost zero.
    return static_cast<uint32_t>(elapsedTime);
}

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
