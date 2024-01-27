#include "json_reader.h"

namespace JSON {

    Reader::Reader(std::istream& input) {
        Node root = Load(input).GetRoot();

        if (!root.IsMap()) {
            return;
        }

        const auto& data = root.AsMap();
        if (data.count("base_requests")) {
            base_request_reader_ = BaseRequestReader(data.at("base_requests"));
        }

        if (data.count("render_settings")) {
            render_settings_reader_ = RendererSettingsReader(data.at("render_settings"));
        }

        if (data.count("stat_requests")) {
            stat_request_reader_ = StatRequestReader(data.at("stat_requests"));
        }
    }

	BaseRequestReader Reader::GetBaseRequestReader() const
	{
		return base_request_reader_;
	}

	StatRequestReader Reader::GetStatRequestReader() const
	{
		return stat_request_reader_;
	}

	RendererSettingsReader Reader::GetRendererSettingsReader() const
	{
		return render_settings_reader_;
	}

	BaseRequestReader::BaseRequestReader(const::JSON::Node& node)
	{
		if (!node.IsArray())
			return;

		const auto& requests = node.AsArray();

		for (const auto& request : requests)
			ParseRequest(request);
	}

	TransportCatalogue::BaseRequests BaseRequestReader::GetBaseRequests() const
	{
		return requests_;
	}

	void BaseRequestReader::ParseRequest(const::JSON::Node& node)
	{
		if (!node.IsMap())
			return;

		const auto& request = node.AsMap();
		const auto& type = request.at("type").AsString();

		if (type == "Stop")
			ParseStopRequest(request);
		else if (type == "Bus")
			ParseBusRequest(request);
	}

	void BaseRequestReader::ParseStopRequest(const::JSON::Dict& stop_request)
	{
		TransportCatalogue::BaseStopRequest request;
		request.name = stop_request.at("name").AsString();
		request.coordinates.lat = stop_request.at("latitude").AsDouble();
		request.coordinates.lng = stop_request.at("longitude").AsDouble();

		const auto& road_distances = stop_request.at("road_distances").AsMap();
		for (const auto& [name, distance] : road_distances)
			request.distances.push_back(TransportCatalogue::StopDistance{ name, distance.AsInt() });

		requests_.push_back(std::move(request));
	}

	void BaseRequestReader::ParseBusRequest(const::JSON::Dict& bus_request)
	{
		TransportCatalogue::BaseBusRequest request;
		request.name = bus_request.at("name").AsString();
		const auto& stops = bus_request.at("stops").AsArray();
		for (const auto& stop : stops)
			request.stops.push_back(stop.AsString());
		request.roundtrip = bus_request.at("is_roundtrip").AsBool();

		requests_.push_back(std::move(request));
	}

	StatRequestReader::StatRequestReader(const::JSON::Node& node)
	{
		if (!node.IsArray())
			return;

		const auto& stat_requests = node.AsArray();

		for (const auto& request : stat_requests)
			ParseRequest(request);
	}

	TransportCatalogue::StatRequests StatRequestReader::GetStatRequests() const
	{
		return requests_;
	}

	void StatRequestReader::ParseRequest(const::JSON::Node& node)
	{
		if (!node.IsMap())
			return;

		const auto& request = node.AsMap();
		const auto& type = request.at("type").AsString();

		if (type == "Bus")
			ParseBusRequest(request);
		else if (type == "Stop")
			ParseStopRequest(request);
		else if (type == "Map")
			ParseMapRequest(request);
	}

	void StatRequestReader::ParseBusRequest(const::JSON::Dict& node)
	{
		TransportCatalogue::StatBusRequest request{ node.at("id").AsInt(), node.at("name").AsString() };
		requests_.push_back(request);
	}

	void StatRequestReader::ParseStopRequest(const::JSON::Dict& node)
	{
		TransportCatalogue::StatStopRequest request{ node.at("id").AsInt(), node.at("name").AsString() };
		requests_.push_back(request);
	}

	void StatRequestReader::ParseMapRequest(const::JSON::Dict& node)
	{
		TransportCatalogue::StatMapRequest request{ node.at("id").AsInt() };
		requests_.emplace_back(request);
	}

	Document ParseResponses(const std::vector<ResponseType>& responses)
	{
		Array array;

		for (const auto& response : responses)
			array.push_back(ParseResponse(response).GetRoot());

		Node node(array);
		return Document(node);
	}

	Document ParseResponse(const ResponseType& response)
	{
		if (std::holds_alternative<TransportCatalogue::Response<TransportCatalogue::StopResponse>>(response))
			return StopResponse(std::get<TransportCatalogue::Response<TransportCatalogue::StopResponse>>(response));

		if (std::holds_alternative<TransportCatalogue::Response<TransportCatalogue::BusResponse>>(response))
			return BusResponse(std::get<TransportCatalogue::Response<TransportCatalogue::BusResponse>>(response));

		if (std::holds_alternative<TransportCatalogue::Response<TransportCatalogue::MapResponse>>(response))
			return MapResponse(std::get<TransportCatalogue::Response<TransportCatalogue::MapResponse>>(response));
		
		throw std::invalid_argument("Unknown request type!");
	}

	Document StopResponse(const TransportCatalogue::Response<TransportCatalogue::StopResponse>& response)
	{		
		if (!response.response.has_value())
			return NotFoundError(response.id);
		
		Dict root;
		root["request_id"] = response.id;

		Array buses;
		for (const auto& bus : response.response->result)
			buses.push_back(bus);
		root["buses"] = buses;
		
		Node node(root);
		return Document(node);
	}

	Document BusResponse(const TransportCatalogue::Response<TransportCatalogue::BusResponse>& response)
	{
		if (!response.response.has_value())
			return NotFoundError(response.id);
		
		Dict root;
		root["request_id"] = response.id;
		root["curvature"] = response.response->curvature;
		root["route_length"] = response.response->distance;
		root["stop_count"] = response.response->stops_count;
		root["unique_stop_count"] = response.response->unique_stops_count;
		
		Node node(root);
		return Document(node);
	}

	Document MapResponse(const TransportCatalogue::Response<TransportCatalogue::MapResponse>& response)
	{
		using namespace std::string_literals;
		Dict root;
		root["request_id"] = response.id;
		root["map"] = response.response->response;
		Node node(root);
		return Document(node);
	}

	Document NotFoundError(int id)
	{
		using namespace std::string_literals;
		Dict root;
		root["request_id"] = id;
		root["error_message"] = "not found"s;
		Node node(root);
		return Document(node);
	}

	RendererSettingsReader::RendererSettingsReader(const Node& node)
	{
		if (!node.IsMap())
			return;

		const auto& settings = node.AsMap();

		settings_.width = settings.at("width").AsDouble();
		settings_.height = settings.at("height").AsDouble();
		settings_.padding = settings.at("padding").AsDouble();
		settings_.stop_radius = settings.at("stop_radius").AsDouble();
		settings_.line_width = settings.at("line_width").AsDouble();
		settings_.bus_label_font_size = settings.at("bus_label_font_size").AsInt();
		
		settings_.bus_label_offset = GetOffset(settings.at("bus_label_offset"));

		settings_.stop_label_font_size = settings.at("stop_label_font_size").AsInt();

		settings_.stop_label_offset = GetOffset(settings.at("stop_label_offset"));

		settings_.underlayer_color = GetColor(settings.at("underlayer_color"));

		settings_.underlayer_width = settings.at("underlayer_width").AsDouble();

		for (const auto& color : settings.at("color_palette").AsArray())
			settings_.color_palette.push_back(GetColor(color));
	}

	Renderer::RenderSettings RendererSettingsReader::GetRenderSettings() const
	{
		return settings_;
	}

	svg::Color RendererSettingsReader::GetColor(const Node& node) const
	{
		if (node.IsArray())
		{
			const auto& color = node.AsArray();
			if (color.size() == 3)
				return svg::Rgb(color[0].AsInt(), color[1].AsInt(), color[2].AsInt());
			else
				return svg::Rgba(color[0].AsInt(), color[1].AsInt(), color[2].AsInt(), color[3].AsDouble());
		}
		else
			return node.AsString();
	}

	Renderer::Offset RendererSettingsReader::GetOffset(const Node& node) const
	{
		const auto& offset = node.AsArray();
		return { offset.at(0).AsDouble(), offset.at(1).AsDouble() };
	}
}