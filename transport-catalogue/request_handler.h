#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <variant>
#include <algorithm>
#include <map>
#include <unordered_set>
#include <set>
#include <sstream>

namespace TransportCatalogue
{
	struct StopDistance
	{
		std::string name;
		int distance;
	};

	struct BaseStopRequest
	{
		std::string name;
		Geo::Coordinates coordinates;
		std::vector<StopDistance> distances;
	};

	struct BaseBusRequest
	{
		std::string name;
		std::vector<std::string> stops;
		bool roundtrip;
	};

	using BaseRequest = std::variant<BaseStopRequest, BaseBusRequest>;
	using BaseRequests = std::vector<BaseRequest>;

	// TODO: Выделить запросы в шаблон по аналогии с Response
	struct StatBusRequest
	{
		int id;
		std::string name;
	};

	struct StatStopRequest
	{
		int id;
		std::string name;
	};

	struct StatMapRequest
	{
		int id;
	};

	using StatRequest = std::variant<StatBusRequest, StatStopRequest, StatMapRequest>;
	using StatRequests = std::vector<StatRequest>;

	template <class ResponseType>
	struct Response
	{
		int id;
		std::optional<ResponseType> response;
	};

	struct MapResponse
	{
		std::string response;
	};

	using VariadicResponse = std::variant<Response<BusResponse>, Response<StopResponse>, Response<MapResponse>>;

	struct LexicalComparator
	{
		bool operator()(const Stop* lhs, const Stop* rhs) const
		{
			return lhs->name < rhs->name;
		}
	};

	class RequestHandler
	{
	public:
		RequestHandler(TransportCatalogue& transport_catalogue, const Renderer::MapRenderer& map_renderer);

		void ProcessRequests(BaseRequests requests);

		std::vector<VariadicResponse> GetResponse(const StatRequests& requests);
		VariadicResponse GetResponse(const StatRequest& request);
		Response<StopResponse> GetStopResponse(const StatStopRequest& request);
		Response<BusResponse> GetBusResponse(const StatBusRequest& request);
		Response<MapResponse> GetMapResponse(const StatMapRequest& request);
		void RenderRoutes(std::ostream& output);
	private:
		void ProcessStopRequests(const std::vector<BaseStopRequest>& stop_requests);
		void ProcessBusRequests(const std::vector<BaseBusRequest>& bus_requests);

		using OrderedBusMap = std::map<std::string_view, const Bus*>;
		svg::Document CreateMap(const OrderedBusMap& bus_to_route_stops) const;

		void CreateRouteLines(svg::Document& document, const OrderedBusMap& bus_to_route_stops) const;

		using StopsInfo = std::pair<std::vector<const Stop*>, bool>;
		using OrderedBusMapToUniqueStop = std::map<std::string_view, StopsInfo>;
		void CreateRouteName(svg::Document& document, const OrderedBusMapToUniqueStop& stops) const;

		using OrderedUniqueStops = std::set<const Stop*, LexicalComparator>;
		void CreateStopCircles(svg::Document& document, const OrderedUniqueStops& stops) const;
		void CreateStopNames(svg::Document& document, const OrderedUniqueStops& stops) const;

		template<class Type>
		std::vector<Type> FilterBy(const BaseRequests& requests) const;
	private:
		TransportCatalogue& tp_;
		Renderer::MapRenderer map_renderer_;
	};

	template <class Type>
	std::vector<Type> RequestHandler::FilterBy(const BaseRequests& requests) const
	{
		std::vector<Type> result;
		std::for_each(requests.cbegin(), requests.cend(),
			[&result](const BaseRequest& request)
			{
				if (std::holds_alternative<Type>(request))
					result.push_back(std::get<Type>(request));
			});

		return result;
	}
}

