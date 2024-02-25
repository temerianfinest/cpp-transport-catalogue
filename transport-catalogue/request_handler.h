#pragma once
#include "transport_catalogue.h"
#include "json_reader.h"
#include <variant>
#include <memory>

namespace request_handler {
    class RequestHandler {
    public:
        using TransportCatalogue = transport_catalogue::TransportCatalogue;

        RequestHandler(const TransportCatalogue &db, const std::vector<std::pair<int, std::string>>& requests);
        RequestHandler(const TransportCatalogue &db, const std::vector<std::pair<int, std::string>>& requests, RendererSettings renderer_settings, const TransportRouter& router);

        // Возвращает информацию о маршруте (запрос Bus)
        BusRoute GetBusStat(const std::string_view &bus_name) const;

        // Возвращает маршруты, проходящие через
        StopRoutes GetBusesByStop(const std::string_view &stop_name) const;

        //Возвращает словарь ответов
        const std::vector<std::pair<int, std::variant<BusRoute, StopRoutes, svg::Document, BusTripRoute>>>& GetAnswers() const;

        //Возвращает список непустых маршрутов
        std::map<std::string_view, std::shared_ptr<Bus>> GetActiveBuses();

        //Загружает объект MapRenderer


    private:
        const TransportCatalogue &db_;
        const std::vector<std::pair<int, std::string>>& requests_;
        std::vector<std::pair<int, std::variant<BusRoute, StopRoutes, svg::Document, BusTripRoute>>> answers_;
        RendererSettings renderer_settings_;
        TransportRouter router_;
    };
}//namespace request_handler