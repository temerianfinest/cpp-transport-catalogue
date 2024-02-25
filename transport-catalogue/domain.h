#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <set>

struct Stop {
    std::string_view stop_name;
    double latitude = 0.0;
    double longitude = 0.0;
    std::string_view next_stops; // Поле для временного хранения строки со следующими остановками
    std::unordered_map<std::string_view, std::uint32_t> dist_to_next;
};

struct Bus {
    std::string_view bus_name;
    std::vector<const Stop *> route;
    bool is_circle = false;
    double r_length = 0.0;
    double true_length = 0.0;
    double curvature = 0.0;
};

struct BusRoute {
    std::string bus_name;
    size_t stops = 0;
    size_t unique_stops = 0;
    double true_length = 0.0;
    double curvature = 0.0;
    bool is_found = false;
};

struct StopRoutes {
    std::string stop_name;
    std::set<std::string_view> routes;
    bool is_found = false;
};