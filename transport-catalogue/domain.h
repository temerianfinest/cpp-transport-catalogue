#pragma once
#include <string>
#include <vector>
#include <set>

#include "geo.h"

namespace TransportCatalogue
{
	struct Stop
	{
		std::string name;
		Geo::Coordinates coordinates;
	};

	struct Bus
	{
		std::string name;
		std::vector<const Stop*> route;
		bool roundtrip;
	};

	struct BusResponse
	{
		int stops_count;
		int unique_stops_count;
		double distance;
		double curvature;
	};


	struct StopResponse
	{
		std::set<std::string> result;
	};
}