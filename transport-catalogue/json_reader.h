#pragma once

#include "json.h"
#include "domain.h"
#include "map_renderer.h"
#include "transport_router.h"
#include <deque>
#include <unordered_map>

namespace json_reader {
    class JsonReader {
    public:
        JsonReader() = default;
        explicit JsonReader(const json::Document& document);

        const std::deque<std::string>& BaseRequestsReturn();
        const std::vector<std::pair<int, std::string>>& StatRequestsReturn();

        const RendererSettings& RenderSettingsReturn();
        const RouterSettings& RouterSettingsReturn();

        json::Document MakeJSON(const std::vector<std::pair<int, std::variant<BusRoute, StopRoutes, svg::Document, BusTripRoute>>>& answers);

    private:
        std::deque<std::string> base_request_;
        std::vector<std::pair<int, std::string>> stat_request_;
        RendererSettings render_settings_;
        RouterSettings router_settings_;

        void JsonBaseReader(const json::Array& base);
        void JsonStatReader(const json::Array& stat);
        void JsonRenderSettingsReader(const json::Dict & settings);
        void JsonRouterSettingsReader(const json::Dict& settings);

        svg::Color ReadColor(const json::Node& node);

        std::string JsonStopToString(const json::Dict& json_stop);
        std::string JsonBusToString(const json::Dict& json_bus);
    };
}//namespace json_reader