/**
 * @file main.cpp
 * @brief Main function for the Netflow v5 exporter assignment
 */

#include <arpa/inet.h>

#include <cstdlib>  // For atoi
#include <iostream>
#include <string>

#include "../include/Flow.h"

struct Arguments {
    std::string hostname;
    std::string port;
    std::string pcap_file;
    int active_timeout = 60;
    int inactive_timeout = 60;
};

/**
 * @brief Error printing function
 */
void print_err() {
    std::cerr << "Usage: ./p2nprobe <host>:<port> <pcap_file_path> [-a <active_timeout> -i <inactive_timeout>]\n";
}

/**
 * @brief Helper function to correctly parse program arguments
 *
 * @param argc 
 * @param argv 
 * @param args Argument structure to hold the argument information
 * @return 
 */
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
            args->port = current_arg.substr(colonPos + 1);
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

int main(int argc, char *argv[]) {
    Arguments args;
    if (!parse_arguments(argc, argv, &args)) {
        print_err();
        return EXIT_FAILURE;
    }

    std::cout << "hostname is: " << args.hostname << std::endl;
    std::cout << "port is: " << args.port << std::endl;
    std::cout << "PCAP FILE is: " << args.pcap_file << std::endl;
    std::cout << "active_timeout is: " << args.active_timeout << std::endl;
    std::cout << "inactive_timeout is: " << args.inactive_timeout << std::endl;

    return 0;
}
