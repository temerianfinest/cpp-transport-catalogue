// main.cpp
#include <iostream>
#include <string>
#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

int main() {
    TransportSystem::TransportCatalogue catalogue;

    TransportSystem::InputReader reader(std::cin); 
    
    int base_request_count;
    std::cin >> base_request_count >> std::ws;

    for (int i = 0; i < base_request_count; ++i) {
        std::string line;
        std::getline(std::cin, line);
        reader.ParseLine(line, catalogue);
    }

    TransportSystem::StatReader stat_reader(std::cin, std::cout);
    
    int stat_request_count;
    std::cin >> stat_request_count >> std::ws;
    for (int i = 0; i < stat_request_count; ++i) {
        std::string line;
        std::getline(std::cin, line);
        stat_reader.ParseAndPrintStat(catalogue, line);
    }

    return 0;
}
