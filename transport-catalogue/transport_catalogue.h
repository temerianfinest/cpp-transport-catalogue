#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <vector>
#include <utility>

#include "geo.h"

namespace TransportSystem {

struct Stop {
    Coordinates coordinates;
};

struct BusRoute {
    std::vector<std::string> stops;
    bool is_circular;
    std::unordered_set<std::string_view> unique_stops;
};
    
    struct PairHash {
    size_t operator()(const std::pair<std::string, std::string>& pair) const {
        return std::hash<std::string>()(pair.first) ^ std::hash<std::string>()(pair.second);
    }
};

class TransportCatalogue {
public:

    void AddStop(const std::string& name, Coordinates coordinates);
    void AddBusRoute(const std::string& name, const std::vector<std::string>& stops);

    const Stop* GetStop(const std::string& name) const;
    const BusRoute* GetBusRoute(const std::string& name) const;

    std::string GetBusRouteInfo(const std::string& name) const;
    std::vector<std::string> GetBusesAtStop(const std::string& stop_name) const;
    
    void SetDistanceBetweenStops(const std::string& stop1, const std::string& stop2, int distance);
    int GetDistanceBetweenStops(const std::string& stop1, const std::string& stop2) const;
    
    double ComputeGeographicalRouteLength(const std::vector<std::string>& stops) const;

private:
    std::unordered_map<std::string, Stop> stops_;
    std::unordered_map<std::string, BusRoute> bus_routes_;
    std::unordered_set<std::string> stop_names_;
    std::deque<std::string> bus_order_;
    std::unordered_map<std::pair<std::string, std::string>, int, PairHash> distances_;

    bool IsCircularRoute(const std::vector<std::string>& stops) const;
    double ComputeRouteLength(const std::vector<std::string>& stops) const;
    
};

} // namespace TransportSystem
