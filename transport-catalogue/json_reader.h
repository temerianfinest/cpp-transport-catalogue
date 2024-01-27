#pragma once

#include "json.h"
#include "request_handler.h"

namespace JSON {

    class BaseRequestReader {
    public:
        BaseRequestReader() = default;
        explicit BaseRequestReader(const Node& node);

        TransportCatalogue::BaseRequests GetBaseRequests() const;

    private:
        void ParseRequest(const Node& node);
        void ParseStopRequest(const Dict& stop_request);
        void ParseBusRequest(const Dict& bus_request);

        TransportCatalogue::BaseRequests requests_;
    };

    class StatRequestReader {
    public:
        StatRequestReader() = default;
        explicit StatRequestReader(const Node& node);

        TransportCatalogue::StatRequests GetStatRequests() const;

    private:
        void ParseRequest(const Node& node);
        void ParseBusRequest(const Dict& node);
        void ParseStopRequest(const Dict& node);
        void ParseMapRequest(const Dict& node);

        TransportCatalogue::StatRequests requests_;
    };

    class RendererSettingsReader {
    public:
        RendererSettingsReader() = default;
        explicit RendererSettingsReader(const Node& node);

        Renderer::RenderSettings GetRenderSettings() const;

    private:
        svg::Color GetColor(const Node& node) const;
        Renderer::Offset GetOffset(const Node& node) const;

        Renderer::RenderSettings settings_;
    };

    class Reader {
    public:
        explicit Reader(std::istream& input);

        BaseRequestReader GetBaseRequestReader() const;
        StatRequestReader GetStatRequestReader() const;
        RendererSettingsReader GetRendererSettingsReader() const;

    private:
        BaseRequestReader base_request_reader_;
        StatRequestReader stat_request_reader_;
        RendererSettingsReader render_settings_reader_;
    };

    using HandlerStopResponse = TransportCatalogue::Response<TransportCatalogue::StopResponse>;
    using HandlerBusResponse = TransportCatalogue::Response<TransportCatalogue::BusResponse>;
    using HandlerMapResponse = TransportCatalogue::Response<TransportCatalogue::MapResponse>;

    using ResponseType = std::variant<HandlerBusResponse, HandlerStopResponse, HandlerMapResponse>;

    Document ParseResponses(const std::vector<ResponseType>& responses);
    Document ParseResponse(const ResponseType& response);
    Document StopResponse(const TransportCatalogue::Response<TransportCatalogue::StopResponse>& response);
    Document BusResponse(const TransportCatalogue::Response<TransportCatalogue::BusResponse>& response);
    Document MapResponse(const TransportCatalogue::Response<TransportCatalogue::MapResponse>& response);
    Document NotFoundError(int id);
}
