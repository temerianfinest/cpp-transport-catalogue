#include "transport_catalogue.h"

#include <algorithm>
#include <unordered_set>
#include <numeric>
#include <iterator>

namespace TransportCatalogue
{
	void TransportCatalogue::AddStop(const Stop& stop)
	{
		auto& it = stops_.emplace_back(stop);
		stopname_to_stop_[it.name] = &it;
		stop_to_busname_[&it];
	}

	void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string_view>& stops, bool roundtrip)
	{
		Bus bus;
		bus.name = name;
		bus.roundtrip = roundtrip;
		for (const auto& stopname : stops)
		{
			const auto stop = stopname_to_stop_[stopname];
			bus.route.push_back(stop);
			stop_to_busname_[stop].insert(name);
		}
		auto& ref = buses_.emplace_back(std::move(bus));
		busname_to_route_[ref.name] = &ref;
	}

	void TransportCatalogue::SetNearbyStop(const std::string_view stop, const std::string_view nearby, int distance)
	{
		const auto stop_ptr = stopname_to_stop_.at(stop);
		const auto nearby_ptr = stopname_to_stop_.at(nearby);

		nearby_stops_to_distance_[{ stop_ptr, nearby_ptr }] = distance;
	}

	std::optional<BusResponse> TransportCatalogue::GetBusInfo(const std::string_view name) const
	{
		if (!busname_to_route_.count(name))
			return std::nullopt;

		BusResponse response;

		const auto bus = busname_to_route_.at(name);
		response.stops_count = static_cast<int>(bus->roundtrip ? bus->route.size() : bus->route.size() * 2 - 1);

		std::unordered_set<const Stop*> unique_stops(bus->route.cbegin(), bus->route.cend());
		response.unique_stops_count = static_cast<int>(unique_stops.size());

		response.distance = GetActualDistance(bus);
		response.curvature = response.distance / GetGeographicDistance(bus);

		return response;
	}

	std::optional<StopResponse> TransportCatalogue::GetStopInfo(const std::string_view stop) const
	{
		if (!stopname_to_stop_.count(stop))
			return std::nullopt;

		return StopResponse{ stop_to_busname_.at(stopname_to_stop_.at(stop)) };
	}

	std::unordered_map<std::string_view, const Bus*> TransportCatalogue::GetRoutes() const
	{
		return busname_to_route_;
	}

	double TransportCatalogue::GetActualDistance(const Bus* bus) const
	{
		double distance = 0;
		for (auto it = bus->route.cbegin(); it != bus->route.cend(); ++it)
			distance += std::next(it) == bus->route.end() ? 0 : GetDistanceBetweenStops(*it, *std::next(it));

		if (bus->roundtrip)
			return distance;

		for (auto it = bus->route.rbegin(); it != bus->route.rend(); ++it)
			distance += std::next(it) == bus->route.rend() ? 0 : GetDistanceBetweenStops(*it, *std::next(it));

		return distance;
	}

	double TransportCatalogue::GetGeographicDistance(const Bus* bus) const
	{
		double distance = 0.0;
		for (auto it = bus->route.cbegin(); it != bus->route.cend(); ++it)
			distance += std::next(it) == bus->route.end() ? 0.0 : ComputeDistance((*it)->coordinates, (*std::next(it))->coordinates);

		return bus->roundtrip ? distance : distance * 2;
	}

	int TransportCatalogue::GetDistanceBetweenStops(const Stop* stop, const Stop* nearby) const
	{
		if (nearby_stops_to_distance_.count({stop, nearby}))
			return nearby_stops_to_distance_.at({stop, nearby});

		return nearby_stops_to_distance_.at({nearby, stop});
	}
}