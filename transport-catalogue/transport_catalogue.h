#pragma once

#include <deque>
#include <string>
#include <unordered_map>
#include <set>
#include <vector>
#include <optional>

#include "domain.h"

namespace transport_catalogue
{
	struct PointerPairHasher
	{
		size_t operator()(const std::pair<const Stop*, const Stop*>& stops) const
		{
			return hasher_(stops.first) * 37 + hasher_(stops.second) * 37 * 37;
		}
	private:
		std::hash<const void*> hasher_;
	};

	class TransportCatalogue
	{
		using NearbyStops = std::pair<const Stop*, const Stop*>;
	public:
		void AddStop(const Stop& stop);
		void AddBus(const std::string& name, const std::vector<std::string_view>& stops, bool roundtrip);

		void SetNearbyStop(const std::string_view stop, const std::string_view nearby, int distance);

		std::optional<BusResponse> GetBusInfo(const std::string_view name) const;
		std::optional<StopResponse> GetStopInfo(const std::string_view stop) const;

		std::unordered_map<std::string_view, const Bus*> GetRoutes() const;
	private:
		int GetDistanceBetweenStops(const Stop* stop, const Stop* nearby) const;
		double GetActualDistance(const Bus* bus) const;
		double GetGeographicDistance(const Bus* bus) const;
	private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;

		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, const Bus*> busname_to_route_;

		std::unordered_map<const Stop*, std::set<std::string>> stop_to_busname_;

		std::unordered_map<NearbyStops, int, PointerPairHasher> nearby_stops_to_distance_;
	};
}