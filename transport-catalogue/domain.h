#pragma once
#include <string>
#include <vector>
#include <set>

#include "geo.h"

namespace transport_catalogue
{
	struct Stop
	{
		std::string name;
		geo::Coordinates coordinates;
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

	//using StopResponse = std::set<std::string>;

	struct StopResponse
	{
		std::set<std::string> result;
	};
}