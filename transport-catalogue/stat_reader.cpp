#include "stat_reader.h"
#include <iomanip>
#include <cmath>

namespace TransportSystem {

StatReader::StatReader(std::istream& input_stream, std::ostream& output_stream)
    : input_stream_(input_stream), output_stream_(output_stream) {}

double StatReader::CalculateCurvature(const TransportCatalogue& transport_catalogue, const std::string& bus_name) const {
    const BusRoute* route = transport_catalogue.GetBusRoute(bus_name);
    if (!route) {
        return 0;
    }

    double actual_length = 0.0;
    for (size_t i = 1; i < route->stops.size(); ++i) {
        actual_length += transport_catalogue.GetDistanceBetweenStops(route->stops[i - 1], route->stops[i]);
    }

    double geo_length = transport_catalogue.ComputeGeographicalRouteLength(route->stops);

    if (geo_length == 0) {
        return 0;
    }

    return actual_length / geo_length;
}

void StatReader::ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request) {
    std::string request_str = std::string(request);

    if (request_str.find("Bus") == 0) {
        std::string bus_name = request_str.substr(4); 
        const BusRoute* route = transport_catalogue.GetBusRoute(bus_name);

        if (!route) {
            output_stream_ << "Bus " << bus_name << ": not found\n";
            return;
        }

        std::string result = transport_catalogue.GetBusRouteInfo(bus_name);
        double curvature = CalculateCurvature(transport_catalogue, bus_name);

        output_stream_ << result << ", " << std::fixed << std::setprecision(5) << curvature << " curvature \n";
    } else if (request_str.find("Stop") == 0) {
        std::string stop_name = request_str.substr(5); 
        const Stop* stop = transport_catalogue.GetStop(stop_name);

        if (stop) {
            output_stream_ << "Stop " << stop_name << ":";
            const auto& buses = transport_catalogue.GetBusesAtStop(stop_name);
            if (buses.empty()) {
                output_stream_ << " no buses \n";
            } else {
                output_stream_ << " buses";
                for (const auto& bus : buses) {
                    output_stream_ << " " << bus;
                }
                output_stream_ << " \n";
            }
        } else {
            output_stream_ << "Stop " << stop_name << ": not found \n";
        }
    }
}

} // namespace TransportSystem
