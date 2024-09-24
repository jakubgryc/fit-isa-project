#include <iostream>
#include <string>
#include <cstdlib> // For atoi

int main(int argc, char *argv[]) {
    // Check if the mandatory arguments <host>:<port> and <pcap_file_path> are provided
    if (argc < 3) {
        std::cerr << "Usage: ./p2nprobe <host>:<port> <pcap_file_path> [-a <active_timeout> -i <inactive_timeout>]\n";
        return 1;
    }

    // Mandatory arguments
    std::string host_port = argv[1];
    std::string pcap_file_path = argv[2];

    // Default values for optional parameters
    int active_timeout = 60;
    int inactive_timeout = 60;

    // Parse optional arguments (-a and -i)
    for (int i = 3; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-a" && i + 1 < argc) {
            active_timeout = std::atoi(argv[++i]); // Convert the next argument to an integer
        } else if (arg == "-i" && i + 1 < argc) {
            inactive_timeout = std::atoi(argv[++i]); // Convert the next argument to an integer
        }
    }

    // Output the parsed values
    std::cout << "Host:Port: " << host_port << "\n";
    std::cout << "PCAP File Path: " << pcap_file_path << "\n";
    std::cout << "Active Timeout: " << active_timeout << " seconds\n";
    std::cout << "Inactive Timeout: " << inactive_timeout << " seconds\n";

    // Proceed with the rest of the program logic using the parsed arguments
    return 0;
}
