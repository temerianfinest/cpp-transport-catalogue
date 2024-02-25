#pragma once
#include "transport_catalogue.h"
#include "router.h"
#include <memory>
#include <variant>

struct RouterSettings{
    int bus_wait_time_ = 1;
    double bus_velocity_ = 1.0;
};

struct BusTripEdges{
    std::string_view bus_name_;
    double time_;
    uint32_t span_count_;
    std::pair<std::string_view, std::string_view> stops_;
};

struct BusTripRoute{
    double total_time_ = 0.0;
    std::vector<BusTripEdges> stages_;
    bool is_found = false;
};


class TransportRouter{
public:
    TransportRouter(const transport_catalogue::TransportCatalogue& tc, RouterSettings router_settings);
    BusTripRoute GetRoute(std::string_view first_stop, std::string_view last_stop);

private:
    const transport_catalogue::TransportCatalogue& tc_;
    RouterSettings router_settings_;
    std::unordered_map<std::string_view, uint32_t> stop_ids_;
    graph::DirectedWeightedGraph<double> graph_;
    std::shared_ptr<graph::Router<double>> route_;
    std::unordered_map<uint32_t, BusTripEdges> edges_ids_;

    void SetEdges();
};