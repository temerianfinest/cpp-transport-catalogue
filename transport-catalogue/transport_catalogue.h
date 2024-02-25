#pragma once
#include "geo.h"
#include <unordered_map>
#include <string_view>
#include <string>
#include <deque>
#include <vector>
#include <functional>
#include <set>
#include <cstdint>
#include "domain.h"
#include <optional>

namespace transport_catalogue {

    namespace detail {
        std::string_view FindName(std::string_view &sv, char separator);
    }

    class TransportCatalogue {
    public:
        TransportCatalogue() = default;
        explicit TransportCatalogue(std::deque<std::string> queries);
        Stop FindStop(std::string_view stop);
        Bus FindBus(std::string_view bus);
        BusRoute RouteInformation(std::string_view bus) const;
        StopRoutes StopInformation(std::string_view stop) const;
        const std::unordered_map<std::string_view, Bus> & GetBuses() const;
        std::optional<uint32_t> GetDistanceBetweenStops(const Stop& lhs, const Stop& rhs) const;
        const std::unordered_map<std::string_view, Stop>& GetStops() const;

    private:
        std::deque<std::string> queries_;
        std::unordered_map<std::string_view, Stop> stops_;
        std::unordered_map<std::string_view, Bus> buses_;
        std::unordered_map<std::string_view, std::set<std::string_view>> buses_for_stops_;

        void AddStop(std::string_view stop_sv);
        void AddNextStops(Stop &stop);
        void AddBus(std::string_view bus_sv);
        void ComputeRealRouteLength(Bus &bus);
    };
}//namespace transport_catalogue