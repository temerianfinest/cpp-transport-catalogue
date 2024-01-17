#include "stat_reader.h"
#include <iomanip>

namespace TransportSystem {

StatReader::StatReader(std::istream& input_stream, std::ostream& output_stream)
    : input_stream_(input_stream), output_stream_(output_stream) {}

void StatReader::ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request) {
    std::string request_str = std::string(request);

    if (request_str.find("Bus") == 0) {
        std::string bus_name = request_str.substr(4); 

        std::string result = transport_catalogue.GetBusRouteInfo(bus_name);

        output_stream_ << result << std::endl;
    } else if (request_str.find("Stop") == 0) {
        std::string stop_name = request_str.substr(5); 
        const Stop* stop = transport_catalogue.GetStop(stop_name);

        if (stop) {
            output_stream_ << "Stop " << stop_name << ":";
            const auto& buses = transport_catalogue.GetBusesAtStop(stop_name);
            if (buses.empty()) {
                output_stream_ << " no buses" << std::endl;
            } else {
                output_stream_ << " buses";
                for (const auto& bus : buses) {
                    output_stream_ << " " << bus;
                }
                output_stream_ << std::endl;
            }
        } else {
            output_stream_ << "Stop " << stop_name << ": not found" << std::endl;
        }
    }
}

} // namespace TransportSystem
