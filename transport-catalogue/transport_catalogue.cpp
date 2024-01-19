#include "transport_catalogue.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <iostream>

namespace TransportSystem {

void TransportCatalogue::AddStop(const std::string& name, Coordinates coordinates) {
    stops_[name] = {coordinates};
    stop_names_.emplace(name);
}

bool TransportCatalogue::IsCircularRoute(const std::vector<std::string>& stops) const {
    return stops.front() == stops.back();
}

void TransportCatalogue::AddBusRoute(const std::string& name, const std::vector<std::string>& stops) {
    BusRoute route;

    route.stops.reserve(stops.size());
    for (const auto& stop : stops) {
        route.stops.push_back(stop);
    }

    route.is_circular = IsCircularRoute(route.stops);

    for (const auto& stop : route.stops) {
        route.unique_stops.insert(stop);
    }

    bus_routes_.emplace(name, std::move(route));
    bus_order_.emplace_back(name);
}

const Stop* TransportCatalogue::GetStop(const std::string& name) const {
    auto it = stops_.find(name);
    return (it != stops_.end()) ? &(it->second) : nullptr;
}

const BusRoute* TransportCatalogue::GetBusRoute(const std::string& name) const {
    auto it = bus_routes_.find(name);
    return (it != bus_routes_.end()) ? &(it->second) : nullptr;
}

std::string TransportCatalogue::GetBusRouteInfo(const std::string& name) const {
    std::ostringstream result;

    const BusRoute* route = GetBusRoute(name);
    if (route) {
        double route_length = 0.0;
        bool reverse_different = false; 

        for (size_t i = 1; i < route->stops.size(); ++i) {
            int distance = GetDistanceBetweenStops(route->stops[i - 1], route->stops[i]);
            
            route_length += distance;
            if (distance != GetDistanceBetweenStops(route->stops[i], route->stops[i - 1])) {
                reverse_different = true;
            }
        }

        if (!route->is_circular && reverse_different) {
            for (size_t i = route->stops.size() - 1; i > 0; --i) {
                route_length += GetDistanceBetweenStops(route->stops[i], route->stops[i - 1]);
            }
        }

        int rounded_route_length = static_cast<int>(std::round(route_length));

        result << "Bus " << name << ": " << route->stops.size() << " stops on route, "
               << route->unique_stops.size() << " unique stops, " << rounded_route_length << " route length";
    } else {
        result << "Bus " << name << ": not found";
    }

    
    
    return result.str();
}

double TransportCatalogue::ComputeRouteLength(const std::vector<std::string>& stops) const {
    double route_length = 0.0;

    for (size_t i = 1; i < stops.size(); ++i) {
        const Stop& stop1 = stops_.at(stops[i - 1]);
        const Stop& stop2 = stops_.at(stops[i]);

        route_length += ComputeDistance(stop1.coordinates, stop2.coordinates);
    }

    return route_length;
}

std::vector<std::string> TransportCatalogue::GetBusesAtStop(const std::string& stop_name) const {
    std::vector<std::string> buses;
    auto it = stop_names_.find(stop_name);

    if (it != stop_names_.end()) {
        for (const auto& bus_name : bus_order_) {
            const BusRoute* route = GetBusRoute(bus_name);
            if (route && route->unique_stops.count(stop_name) > 0) {
                buses.push_back(bus_name);
            }
        }
    }

    std::sort(buses.begin(), buses.end(), [](const std::string& bus1, const std::string& bus2) {
        return bus1 < bus2;
    });

    return buses;
}
    
    void TransportCatalogue::SetDistanceBetweenStops(const std::string& stop1, const std::string& stop2, int distance) {
    auto reverse_distance_it = distances_.find({stop2, stop1});
    if (reverse_distance_it == distances_.end()) {
        
        distances_[{stop2, stop1}] = distance;
    }
    distances_[{stop1, stop2}] = distance;
}

int TransportCatalogue::GetDistanceBetweenStops(const std::string& stop1, const std::string& stop2) const {
    auto it = distances_.find({stop1, stop2});
    if (it != distances_.end()) {
        return it->second;
    }

    it = distances_.find({stop2, stop1});
    return it != distances_.end() ? it->second : -1;
}
    
    double TransportCatalogue::ComputeGeographicalRouteLength(const std::vector<std::string>& stops) const {
    double total_length = 0.0;
    for (size_t i = 1; i < stops.size(); ++i) {
        const Stop& stop1 = stops_.at(stops[i - 1]);
        const Stop& stop2 = stops_.at(stops[i]);
        total_length += ComputeDistance(stop1.coordinates, stop2.coordinates);
    }
    return total_length;
}

} // namespace TransportSystem
