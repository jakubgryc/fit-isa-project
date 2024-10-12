/**
 * @file main.cpp
 * @brief Main function for the Netflow v5 exporter assignment
 */

#include <arpa/inet.h>
#include <sys/socket.h>

#include <cstdlib>  // For atoi
#include <iostream>
#include <string>

#include "../include/Flow.h"
#include "../include/Tools.h"
#include "../include/UDPExporter.h"
#include "../include/PcapHandler.h"


int main(int argc, char *argv[]) {
    Arguments args;
    if (!parse_arguments(argc, argv, &args)) {
        print_err();
        return EXIT_FAILURE;
    }

    UDPExporter *connection = new UDPExporter(args.hostname, args.port);
    Timer timer(args.active_timeout, args.inactive_timeout);

    if (!connection->connect()) {
        std::cerr << "Unable to connect to: " << args.hostname << ":" << args.port << std::endl;
        // delete connection;
        return EXIT_FAILURE;
    }

    // for (int i = 0; i < 7; i++) {
    //     connection->send_flow("NEJAKA DATA\n");
    // }

    PcapHandler pcap_handler(args.pcap_file);

    pcap_handler.openPcap();
    pcap_handler.start(connection, timer);

    // connection->printData();

    // std::cout << "---------------------" << std::endl;
    // std::cout << "hostname is: |" << args.hostname << "|\n";
    // std::cout << "port is: |" << args.port << "|\n";
    std::cout << "PCAP FILE is: " << args.pcap_file << std::endl;
    // std::cout << "active_timeout is: " << args.active_timeout << std::endl;
    // std::cout << "inactive_timeout is: " << args.inactive_timeout << std::endl;
    // std::cout << "---------------------" << std::endl;

    delete connection;

    return 0;
}
