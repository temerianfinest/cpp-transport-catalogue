#include "request_handler.h"
#include <unordered_map>

#include <stdexcept>
#include <iostream>
#include <map>
#include <unordered_set>
#include <set>

namespace TransportCatalogue
{
	RequestHandler::RequestHandler(TransportCatalogue& transport_catalogue, const Renderer::MapRenderer& map_renderer)
		: tp_(transport_catalogue), map_renderer_(map_renderer)
	{ }

	void RequestHandler::ProcessRequests(BaseRequests requests)
	{
		ProcessStopRequests(FilterBy<BaseStopRequest>(requests));
		ProcessBusRequests(FilterBy<BaseBusRequest>(requests));
	}

	std::vector<VariadicResponse> RequestHandler::GetResponse(const StatRequests& requests)
	{
		std::vector<VariadicResponse> responses;
		responses.reserve(requests.size());
		for (const auto& request : requests)
			responses.push_back(GetResponse(request));
		return responses;
	}

	VariadicResponse RequestHandler::GetResponse(const StatRequest& request)
	{
		if (std::holds_alternative<StatBusRequest>(request))
			return GetBusResponse(std::get<StatBusRequest>(request));
		
		if (std::holds_alternative<StatStopRequest>(request))
			return GetStopResponse(std::get<StatStopRequest>(request));

		if (std::holds_alternative<StatMapRequest>(request))
			return GetMapResponse(std::get<StatMapRequest>(request));

		throw std::invalid_argument("Unknown request type");
	}

	Response<BusResponse> RequestHandler::GetBusResponse(const StatBusRequest& request)
	{
		return { request.id, tp_.GetBusInfo(request.name) };
	}

	Response<MapResponse> RequestHandler::GetMapResponse(const StatMapRequest& request)
	{
		std::stringstream response;
		RenderRoutes(response);
		return { request.id,  MapResponse{ response.str() } };
	}

	void RequestHandler::RenderRoutes(std::ostream& output)
	{
		const auto& routes = tp_.GetRoutes();

		OrderedBusMap bus_to_route_stops;
		std::vector<Geo::Coordinates> coordinates;

		for (const auto& [name, bus] : routes)
		{
			if (bus->route.empty())
				continue;

			for (const auto& route : bus->route)
				coordinates.push_back(route->coordinates);

			bus_to_route_stops[name] = bus;
		}

		map_renderer_.SetSphereProjector(coordinates);
		
		CreateMap(bus_to_route_stops).Render(output);
	}

	Response<StopResponse> RequestHandler::GetStopResponse(const StatStopRequest& request)
	{
		return { request.id, tp_.GetStopInfo(request.name) };
	}

	void RequestHandler::ProcessStopRequests(const std::vector<BaseStopRequest>& stop_requests)
	{
		std::unordered_map<std::string_view, std::vector<StopDistance>> stop_distances;
		
		for (const auto& request : stop_requests)
		{
			tp_.AddStop({ request.name, request.coordinates });
			if (!request.distances.empty())
				stop_distances[request.name] = request.distances;
		}
		
		for (const auto& [stop, distances] : stop_distances)
			for (const auto& distance : distances)
				tp_.SetNearbyStop(stop, distance.name, distance.distance);
	}

	void RequestHandler::ProcessBusRequests(const std::vector<BaseBusRequest>& bus_requests)
	{
		for (const auto& request : bus_requests)
			tp_.AddBus(request.name, 
				std::vector<std::string_view>(request.stops.cbegin(), request.stops.cend()),
				request.roundtrip);
	}

	svg::Document RequestHandler::CreateMap(const OrderedBusMap& bus_to_route_stops) const
	{
		svg::Document document;
		
		CreateRouteLines(document, bus_to_route_stops);

		OrderedUniqueStops ordered_unique_stops;
		OrderedBusMapToUniqueStop buses_to_stops_info;
		for (const auto& [name, bus] : bus_to_route_stops)
		{
			StopsInfo info;
			for (const auto& stop : bus->route)
			{
				info.first.push_back(stop);
				info.second = bus->roundtrip;
				ordered_unique_stops.insert(stop);
			}
			buses_to_stops_info[name] = info;
		}

		CreateRouteName(document, buses_to_stops_info);
		CreateStopCircles(document, ordered_unique_stops);
		CreateStopNames(document, ordered_unique_stops);
		return document;
	}

	void RequestHandler::CreateRouteLines(svg::Document& document, const OrderedBusMap& bus_to_route_stops) const
	{
		using namespace std::string_literals;

		const auto& palette = map_renderer_.GetSettings().color_palette;
		auto color = palette.cbegin();
		for (const auto& [name, bus] : bus_to_route_stops)
		{
			auto line = map_renderer_.CreateRouteLine(*color);
			for (const auto& stop : bus->route)
				line.AddPoint(map_renderer_.Project(stop->coordinates));
			
			if (!bus->roundtrip)
				for (auto it = bus->route.rbegin() + 1; it != bus->route.rend(); ++it)
					line.AddPoint(map_renderer_.Project((*it)->coordinates));

			document.Add(line);
			color = Renderer::CyclicIteration(palette, color);
		}
	}

	void RequestHandler::CreateRouteName(svg::Document& document, const OrderedBusMapToUniqueStop& buses_to_stop_info) const
	{
		const auto& palette = map_renderer_.GetSettings().color_palette;
		auto color = palette.cbegin();
		for (const auto& [name, stop_info] : buses_to_stop_info)
		{
			auto text = map_renderer_.CreateRouteName(name.data(), map_renderer_.Project(stop_info.first.front()->coordinates), *color);
			auto mount = map_renderer_.CreateMountForLabel(text);
			document.Add(mount);
			document.Add(text);

			if (!stop_info.second && stop_info.first.front() != stop_info.first.back())
			{
				text.SetPosition(map_renderer_.Project(stop_info.first.back()->coordinates));
				mount.SetPosition(map_renderer_.Project(stop_info.first.back()->coordinates));
				document.Add(mount);
				document.Add(text);
			}
			color = Renderer::CyclicIteration(palette, color);
		}
	}

	void RequestHandler::CreateStopCircles(svg::Document& document, const OrderedUniqueStops& stops) const
	{
		for (const auto& stop : stops)
			document.Add(map_renderer_.CreateStopCircle(map_renderer_.Project(stop->coordinates)));
	}

	void RequestHandler::CreateStopNames(svg::Document& document, const OrderedUniqueStops& stops) const
	{
		for (const auto& stop : stops)
		{
			const auto& text = map_renderer_.CreateStopName(stop->name, map_renderer_.Project(stop->coordinates));
			const auto& mount = map_renderer_.CreateMountForLabel(text);
			document.Add(mount);
			document.Add(text);
		}
	}
}
