#include "stat_reader.h"
#include <iomanip>

namespace TransportSystem {

void StatReader::ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
                                   std::ostream& output) {
    std::string request_str = std::string(request);

    if (request_str.find("Bus") == 0) {

        std::string bus_name = request_str.substr(4); 

        std::string result = transport_catalogue.GetBusRouteInfo(bus_name);

        output << result << std::endl;
    } else if (request_str.find("Stop") == 0) {
        
        std::string stop_name = request_str.substr(5); 

        const Stop* stop = transport_catalogue.GetStop(stop_name);

        if (stop) {
            output << "Stop " << stop_name << ":";
            const auto& buses = transport_catalogue.GetBusesAtStop(stop_name);
            if (buses.empty()) {
                output << " no buses" << std::endl;
            } else {
                output << " buses";
                for (const auto& bus : buses) {
                    output << " " << bus;
                }
                output << std::endl;
            }
        } else {
            output << "Stop " << stop_name << ": not found" << std::endl;
        }
    }
}

} // namespace TransportSystem
