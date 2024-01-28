#include "json_reader.h"

namespace json
{
	Reader::Reader(std::istream& input)
	{
		Node root = Load(input).GetRoot();

		if (!root.IsDict())
			return;

		const auto& data = root.AsDict();
		if (data.count("base_requests"))
			base_request_reader_ = BaseRequestReader(data.at("base_requests"));

		if (data.count("render_settings"))
			render_settings_reader_ = RendererSettingsReader(data.at("render_settings"));

		if (data.count("stat_requests"))
			stat_request_reader_ = StatRequestReader(data.at("stat_requests"));
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

	BaseRequestReader::BaseRequestReader(const Node& node)
	{
		if (!node.IsArray())
			return;

		const auto& requests = node.AsArray();

		for (const auto& request : requests)
			ParseRequest(request);
	}

	transport_catalogue::BaseRequests BaseRequestReader::GetBaseRequests() const
	{
		return requests_;
	}

	void BaseRequestReader::ParseRequest(const Node& node)
	{
		if (!node.IsDict())
			return;

		const auto& request = node.AsDict();
		const auto& type = request.at("type").AsString();

		if (type == "Stop")
			ParseStopRequest(request);
		else if (type == "Bus")
			ParseBusRequest(request);
	}

	void BaseRequestReader::ParseStopRequest(const Dict& stop_request)
	{
		transport_catalogue::BaseStopRequest request;
		request.name = stop_request.at("name").AsString();
		request.coordinates.lat = stop_request.at("latitude").AsDouble();
		request.coordinates.lng = stop_request.at("longitude").AsDouble();

		const auto& road_distances = stop_request.at("road_distances").AsDict();
		for (const auto& [name, distance] : road_distances)
			request.distances.push_back(transport_catalogue::StopDistance{ name, distance.AsInt() });

		requests_.push_back(std::move(request));
	}

	void BaseRequestReader::ParseBusRequest(const Dict& bus_request)
	{
		transport_catalogue::BaseBusRequest request;
		request.name = bus_request.at("name").AsString();
		const auto& stops = bus_request.at("stops").AsArray();
		for (const auto& stop : stops)
			request.stops.push_back(stop.AsString());
		request.roundtrip = bus_request.at("is_roundtrip").AsBool();

		requests_.push_back(std::move(request));
	}

	StatRequestReader::StatRequestReader(const Node& node)
	{
		if (!node.IsArray())
			return;

		const auto& stat_requests = node.AsArray();

		for (const auto& request : stat_requests)
			ParseRequest(request);
	}

	transport_catalogue::StatRequests StatRequestReader::GetStatRequests() const
	{
		return requests_;
	}

	void StatRequestReader::ParseRequest(const Node& node)
	{
		if (!node.IsDict())
			return;

		const auto& request = node.AsDict();
		const auto& type = request.at("type").AsString();

		if (type == "Bus")
			ParseBusRequest(request);
		else if (type == "Stop")
			ParseStopRequest(request);
		else if (type == "Map")
			ParseMapRequest(request);
	}

	void StatRequestReader::ParseBusRequest(const Dict& node)
	{
		transport_catalogue::StatBusRequest request{ node.at("id").AsInt(), node.at("name").AsString() };
		requests_.push_back(request);
	}

	void StatRequestReader::ParseStopRequest(const Dict& node)
	{
		transport_catalogue::StatStopRequest request{ node.at("id").AsInt(), node.at("name").AsString() };
		requests_.push_back(request);
	}

	void StatRequestReader::ParseMapRequest(const Dict& node)
	{
		transport_catalogue::StatMapRequest request{ node.at("id").AsInt() };
		requests_.push_back(request);
	}

	Document ParseResponses(const std::vector<ResponseType>& responses)
	{
		Builder builder;
		builder.StartArray();

		for (const auto& response : responses)
			ParseResponse(response, builder);

		builder.EndArray();
		return Document(builder.Build());
	}

	void ParseResponse(const ResponseType& response, Builder& context)
	{
		using namespace transport_catalogue;
		if (std::holds_alternative<Response<transport_catalogue::StopResponse>>(response))
			StopResponse(std::get<Response<transport_catalogue::StopResponse>>(response), context);
		else if (std::holds_alternative<Response<transport_catalogue::BusResponse>>(response))
			BusResponse(std::get<Response<transport_catalogue::BusResponse>>(response), context);
		else if (std::holds_alternative<Response<transport_catalogue::MapResponse>>(response))
			MapResponse(std::get<Response<transport_catalogue::MapResponse>>(response), context);
	}

	void StopResponse(const HandlerStopResponse& response, Builder& context)
	{	
		if (!response.response.has_value())
		{
			NotFoundError(response.id, context);
			return;
		}
		
		context.StartDict().Key("request_id").Value(response.id).
			Key("buses").StartArray();

		for (const auto& bus : response.response->result)
			context.Value(bus);
		
		context.EndArray();
		context.EndDict();
	}

	void BusResponse(const HandlerBusResponse& response, Builder& context)
	{
		if (!response.response.has_value())
		{
			NotFoundError(response.id, context);
			return;
		}

		context.StartDict().Key("request_id").Value(response.id).
			Key("curvature").Value(response.response->curvature).Key("route_length").Value(response.response->distance).
			Key("stop_count").Value(response.response->stops_count).
			Key("unique_stop_count").Value(response.response->unique_stops_count);

		context.EndDict();
	}

	void MapResponse(const HandlerMapResponse& response, Builder& context)
	{
		using namespace std::string_literals;

		context.StartDict().Key("request_id").Value(response.id).
			Key("map").Value(response.response->response);

		context.EndDict();
	}

	void NotFoundError(int id, Builder& context)
	{
		using namespace std::string_literals;
		
		context.StartDict().Key("request_id").Value(id).
			Key("error_message").Value("not found"s);
		context.EndDict();
	}

	RendererSettingsReader::RendererSettingsReader(const Node& node)
	{
		if (!node.IsDict())
			return;

		const auto& settings = node.AsDict();

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