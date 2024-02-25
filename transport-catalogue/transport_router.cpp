#include "transport_router.h"

TransportRouter::TransportRouter(const transport_catalogue::TransportCatalogue &tc, RouterSettings router_settings) : tc_(tc)
        , router_settings_(router_settings)
        , graph_(tc_.GetStops().size()){
    uint32_t i = 0;
    for(const auto& [name, stop] : tc.GetStops()){
        stop_ids_.insert({name, i});
        ++i;
    }
    SetEdges();
    route_ = std::make_shared<graph::Router<double>>(graph_);
}

void TransportRouter::SetEdges() {
    uint32_t edge_num = 0;
    for(const auto& [name, bus] : tc_.GetBuses()){
        for(auto first_stop = bus.route.begin(); first_stop != bus.route.end(); ++first_stop){
            graph::VertexId first = stop_ids_.at((*first_stop)->stop_name);
            double weight = router_settings_.bus_wait_time_;
            for(auto last_stop = std::next(first_stop); last_stop != bus.route.end(); ++last_stop){
                graph::VertexId last = stop_ids_.at((*last_stop)->stop_name);
                std::optional<uint32_t> dist = tc_.GetDistanceBetweenStops(*(*(std::prev(last_stop))), *(*last_stop));
                if(dist.has_value()){
                    weight += dist.value() / (router_settings_.bus_velocity_ * 1000 / 60);
                    graph_.AddEdge({first, last, weight});
                    edges_ids_.insert({edge_num, {bus.bus_name, weight,
                                                  static_cast<uint32_t>(std::distance(first_stop, last_stop)),
                                                  {(*first_stop)->stop_name, (*last_stop)->stop_name}}});
                    ++edge_num;
                }
            }
        }
        if(!bus.is_circle) {
            for (auto first_stop = bus.route.rbegin(); first_stop != bus.route.rend(); ++first_stop) {
                graph::VertexId first = stop_ids_.at((*first_stop)->stop_name);
                double weight = router_settings_.bus_wait_time_;
                for (auto last_stop = std::next(first_stop); last_stop != bus.route.rend(); ++last_stop) {
                    graph::VertexId last = stop_ids_.at((*last_stop)->stop_name);
                    std::optional<uint32_t> dist = tc_.GetDistanceBetweenStops(*(*(std::prev(last_stop))),
                                                                               *(*last_stop));
                    if (dist.has_value()) {
                        weight += dist.value() / (router_settings_.bus_velocity_ * 1000 / 60);
                        graph_.AddEdge({first, last, weight});
                        edges_ids_.insert({edge_num, {bus.bus_name, weight,
                                                      static_cast<uint32_t>(std::distance(first_stop, last_stop)),
                                                      {(*first_stop)->stop_name, (*last_stop)->stop_name}}});
                        ++edge_num;
                    }
                }
            }
        }
    }
}

BusTripRoute
TransportRouter::GetRoute(std::string_view first_stop, std::string_view last_stop) {
    BusTripRoute route;
    if(!stop_ids_.count(first_stop) || !stop_ids_.count(last_stop)){
        return route;
    }
    auto result = route_->BuildRoute(stop_ids_.at(first_stop), stop_ids_.at(last_stop));
    if(!result.has_value()){
        return route;
    }
    route.is_found = true;
    route.total_time_ = result->weight;
    for(const auto& trip_edge : result.value().edges){
        route.stages_.push_back(edges_ids_.at(trip_edge));
    }
    return route;
}