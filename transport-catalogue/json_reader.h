#pragma once

#include "json_builder.h"
#include "request_handler.h"

namespace json
{
	class BaseRequestReader
	{
	public:
		BaseRequestReader() = default;
		explicit BaseRequestReader(const Node& node);
		transport_catalogue::BaseRequests GetBaseRequests() const;
	private:
		void ParseRequest(const Node& node);
		void ParseStopRequest(const Dict& stop_request);
		void ParseBusRequest(const Dict& bus_request);
	private:
		transport_catalogue::BaseRequests requests_;
	};

	class StatRequestReader
	{
	public:
		StatRequestReader() = default;
		explicit StatRequestReader(const Node& node);
		transport_catalogue::StatRequests GetStatRequests() const;
	private:
		void ParseRequest(const Node& node);
		void ParseBusRequest(const Dict& node);
		void ParseStopRequest(const Dict& node);
		void ParseMapRequest(const Dict& node);
	private:
		transport_catalogue::StatRequests requests_;
	};

	class RendererSettingsReader
	{
	public:
		RendererSettingsReader() = default;
		explicit RendererSettingsReader(const Node& node);
		Renderer::RenderSettings GetRenderSettings() const;
	private:
		svg::Color GetColor(const Node& node) const;
		Renderer::Offset GetOffset(const Node& node) const;
	private:
		Renderer::RenderSettings settings_;
	};

	class Reader
	{
	public:
		Reader(std::istream& input);
		BaseRequestReader GetBaseRequestReader() const;
		StatRequestReader GetStatRequestReader() const;
		RendererSettingsReader GetRendererSettingsReader() const;
	private:
		BaseRequestReader base_request_reader_;
		StatRequestReader stat_request_reader_;
		RendererSettingsReader render_settings_reader_;
	};
	
	using HandlerStopResponse = transport_catalogue::Response<transport_catalogue::StopResponse>;
	using HandlerBusResponse = transport_catalogue::Response<transport_catalogue::BusResponse>;
	using HandlerMapResponse = transport_catalogue::Response<transport_catalogue::MapResponse>;

	using ResponseType = std::variant<HandlerBusResponse, HandlerStopResponse, HandlerMapResponse>;
	
	Document ParseResponses(const std::vector<ResponseType>& responses);

	void ParseResponse(const ResponseType& response, Builder& context);
	void StopResponse(const HandlerStopResponse& response, Builder& context);
	void BusResponse(const HandlerBusResponse& response, Builder& context);
	void MapResponse(const HandlerMapResponse& response, Builder& context);
	void NotFoundError(int id, Builder& context);
}

	