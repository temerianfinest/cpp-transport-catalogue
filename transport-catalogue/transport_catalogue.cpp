#include "transport_catalogue.h"
#include <functional>
#include <cstdlib>
#include <limits>

namespace transport_catalogue {
    namespace {
        void RemoveBeginEndSpaces(std::string_view &str) {
            while (str.front() == ' ') {
                str.remove_prefix(1);
            }
            while (str.back() == ' ') {
                str.remove_suffix(1);
            }
        }
    }//namespace
    
    namespace detail {
        std::string_view FindName(std::string_view &sv, char separator) {
            std::string_view name;
            std::uint64_t sep_pos = 0;
            if (sv.find(separator) != std::string_view::npos) { //Ищем признак окончания названия
                sep_pos = sv.find(separator);
                name = sv.substr(0, sep_pos);
            } else {
                name = sv;
            }
            RemoveBeginEndSpaces(name); //Убираем начальные и конечные пробелы
            if (sep_pos != 0) {
                sv.remove_prefix(sep_pos + 1);
            } else {
                sv.remove_prefix(sv.size());
            }
            return name;
        }
    }//namespace detail

    using detail::FindName;

    TransportCatalogue::TransportCatalogue(std::deque<std::string> queries)
            : queries_(std::move(queries)) {
        using namespace std::string_literals;
        for (const auto &i: queries_) {
            if (i.substr(0, 4) == "Stop"s) {
                AddStop(i);
            }
            if (i.substr(0, 3) == "Bus"s) {
                AddBus(i);
            }
        }
        for (auto &i: stops_) {
            AddNextStops(i.second);
        }
        for (auto &i: buses_) {
            ComputeRealRouteLength(i.second);
        }
    }

    void TransportCatalogue::AddStop(std::string_view stop_sv) {
        Stop stop;
        //Находим имя остановки
        stop_sv.remove_prefix(4); //Убираем слово Stop
        stop.stop_name = FindName(stop_sv, ':');
        //Преобразуем строковые значения широты и долготы в числовые
        std::string_view lat_ = FindName(stop_sv, ',');
        stop.latitude = std::stod({lat_.data(), lat_.size()});
        std::string_view lng_ = FindName(stop_sv, ',');
        stop.longitude = std::stod({lng_.data(), lng_.size()});
        stop.next_stops = stop_sv; //string_view с оставшейся информацией для последующей обработки
        stops_.insert({stop.stop_name, stop});
        buses_for_stops_.insert({stop.stop_name, {}});
    }

    void TransportCatalogue::AddNextStops(Stop &stop) {
        using namespace std::string_literals;
        while (!stop.next_stops.empty()) {
            std::string_view distance = FindName(stop.next_stops, 'm');
            stop.next_stops.remove_prefix(stop.next_stops.find("to"s));
            stop.next_stops.remove_prefix(2);
            std::string_view next_name = FindName(stop.next_stops, ',');
            if (stops_.count(next_name)) {
                stop.dist_to_next.insert(
                        {stops_.at(next_name).stop_name, std::stod({distance.data(), distance.size()})});
            }
        }
    }

    void TransportCatalogue::AddBus(std::string_view bus_sv) {
        Bus bus;
        bus_sv.remove_prefix(3);
        bus.bus_name = FindName(bus_sv, ':');
        char sep = '-';
        if (bus_sv.find('>') != std::string_view::npos) {
            bus.is_circle = true;
            sep = '>';
        }
        while (!bus_sv.empty()) {
            auto stop_name = FindName(bus_sv, sep);
            if(stop_name.size() == std::string_view::npos){
                continue;
            }
            else if (stops_.count(stop_name)) {
                Stop *stop_ptr = &stops_.at(stop_name);
                bus.route.push_back(stop_ptr);
                buses_for_stops_[stop_ptr->stop_name].insert(bus.bus_name);
            }
        }
        if(bus.route.size() > 1) {
            for (size_t i = 1; i < bus.route.size(); ++i) {
                if (bus.route[i - 1] == bus.route[i]) {
                    continue;
                } else {
                    bus.r_length += geo::ComputeDistance({bus.route[i - 1]->latitude, bus.route[i - 1]->longitude},
                                                         {bus.route[i]->latitude, bus.route[i]->longitude});
                }
            }
        }
        if (!bus.is_circle) {
            bus.r_length *= 2;
        }
        buses_.insert({bus.bus_name, bus});
    }

    void TransportCatalogue::ComputeRealRouteLength(Bus &bus) {
        if(bus.route.size() > 1) {
            for (size_t i = 1; i < bus.route.size(); ++i) {
                if (bus.route[i - 1]->dist_to_next.count(bus.route[i]->stop_name)) {
                    bus.true_length += bus.route[i - 1]->dist_to_next.at(bus.route[i]->stop_name);
                } else if (bus.route[i]->dist_to_next.count(bus.route[i - 1]->stop_name)) {
                    bus.true_length += bus.route[i]->dist_to_next.at(bus.route[i - 1]->stop_name);
                }
            }
            if (!bus.is_circle) {
                for (size_t i = 1; i < bus.route.size(); ++i) {
                    if (bus.route[i]->dist_to_next.count(bus.route[i - 1]->stop_name)) {
                        bus.true_length += bus.route[i]->dist_to_next.at(bus.route[i - 1]->stop_name);
                    } else if (bus.route[i - 1]->dist_to_next.count(bus.route[i]->stop_name)) {
                        bus.true_length += bus.route[i - 1]->dist_to_next.at(bus.route[i]->stop_name);
                    }
                }
            }
        }
        if(bus.r_length > std::numeric_limits<double>::epsilon()) {
            bus.curvature = bus.true_length / bus.r_length;
        }
        else{
            bus.curvature = 1;
        }
    }

    Stop TransportCatalogue::FindStop(std::string_view stop) {
        return stops_.at(stop);
    }

    Bus TransportCatalogue::FindBus(std::string_view bus) {
        return buses_.at(bus);
    }

    BusRoute TransportCatalogue::RouteInformation(std::string_view bus) const {
        RemoveBeginEndSpaces(bus);
        BusRoute route;
        if (buses_.count(bus)) {
            route.is_found = true;
            std::set<const Stop *> unique_stops(buses_.at(bus).route.begin(), buses_.at(bus).route.end());
            route.bus_name = {buses_.at(bus).bus_name.data(), buses_.at(bus).bus_name.size()};
            route.stops = (buses_.at(bus).is_circle) ? (buses_.at(bus).route.size()) : (
                    buses_.at(bus).route.size() * 2 - 1);
            route.unique_stops = unique_stops.size();
            route.true_length = buses_.at(bus).true_length;
            route.curvature = buses_.at(bus).curvature;
        } else {
            route.bus_name = {bus.data(), bus.size()};
        }
        return route;
    }

    StopRoutes TransportCatalogue::StopInformation(std::string_view stop) const{
        RemoveBeginEndSpaces(stop);
        StopRoutes buses_for_stop;
        if (buses_for_stops_.count(stop)) {
            buses_for_stop.is_found = true;
            buses_for_stop.stop_name = {stops_.at(stop).stop_name.data(), stops_.at(stop).stop_name.size()};
            buses_for_stop.routes = buses_for_stops_.at(stop);
        } else {
            buses_for_stop.stop_name = {stop.data(), stop.size()};
        }
        return buses_for_stop;
    }

    const std::unordered_map<std::string_view, Bus> & TransportCatalogue::GetBuses() const {
        return buses_;
    }

    std::optional<uint32_t> TransportCatalogue::GetDistanceBetweenStops(const Stop &lhs, const Stop &rhs) const {
        if(lhs.dist_to_next.count(rhs.stop_name)){
            return lhs.dist_to_next.at(rhs.stop_name);
        }
        else if(rhs.dist_to_next.count(lhs.stop_name)){
            return rhs.dist_to_next.at(lhs.stop_name);
        }
        return std::nullopt;
    }

    const std::unordered_map<std::string_view, Stop> &TransportCatalogue::GetStops() const {
        return stops_;
    }
}