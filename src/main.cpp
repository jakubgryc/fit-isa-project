/**
 * @file main.cpp
 * @brief Main function for the Netflow v5 exporter assignment
 */

#include <arpa/inet.h>
#include <sys/socket.h>

#include <iostream>

#include "../include/PcapHandler.h"
#include "../include/Tools.h"
#include "../include/UDPExporter.h"

int main(int argc, char *argv[]) {
    Arguments args;
    if (!parse_arguments(argc, argv, &args)) {
        print_err();
        return EXIT_FAILURE;
    }

    UDPExporter *exporter = new UDPExporter(args.hostname, args.port);
    
    // Create a timer object with the active and inactive timeout values
    // Upon creation, the timer will calculate the current time to be used as the start time
    Timer timer(args.active_timeout, args.inactive_timeout);

    if (!exporter->connect()) {
        std::cerr << "Unable to connect to: " << args.hostname << ":" << args.port << std::endl;
        return EXIT_FAILURE;
    }

    PcapHandler pcap_handler(args.pcap_file);

    pcap_handler.openPcap();
    pcap_handler.start(exporter, timer);

    delete exporter;

    return 0;
}
