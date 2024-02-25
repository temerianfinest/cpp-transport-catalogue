#include "request_handler.h"

#include <utility>

namespace request_handler {
using namespace std::string_literals;

    RequestHandler::RequestHandler(const RequestHandler::TransportCatalogue &db,
                                   const std::vector<std::pair<int, std::string>> &requests,
                                   RendererSettings renderer_settings, const TransportRouter& router)
    : db_(db)
    , requests_(requests)
    , renderer_settings_(std::move(renderer_settings))
    , router_(router){
        for(auto& [id, request] : requests_){
            auto space = request.find_first_of(' ');
            if(request.substr(0, space) == "Bus"s){
                answers_.emplace_back(id, GetBusStat(request.substr(space)));
            }
            else if(request.substr(0, space) == "Stop"s){
                answers_.emplace_back(id, GetBusesByStop(request.substr(space)));
            }
            else if(request.substr(0, space) == "Map"s){
                MapRenderer map_renderer(renderer_settings_, GetActiveBuses());
                answers_.emplace_back(id, map_renderer.RenderMap());
            }
            else if(request.substr(0, space) == "Route"s){
                auto separator = request.find(" -> ", ++space);
                std::string first_stop = request.substr(space, (separator - space));
                std::string last_stop = request.substr(separator + 4);
                answers_.emplace_back(id, router_.GetRoute(first_stop, last_stop));
            }
        }
    }

    // Возвращает информацию о маршруте (запрос Bus)
    BusRoute RequestHandler::GetBusStat(const std::string_view &bus_name) const {
        return db_.RouteInformation(bus_name);
    }
    // Возвращает маршруты, проходящие через остановку (запрос Stop)
    StopRoutes RequestHandler::GetBusesByStop(const std::string_view &stop_name) const {
        return db_.StopInformation(stop_name);
    }
    //Возвращает словарь ответов
    const std::vector<std::pair<int, std::variant<BusRoute, StopRoutes, svg::Document, BusTripRoute>>>& RequestHandler::GetAnswers() const{
        return answers_;
    }
    //Возвращает список непустых маршрутов
    std::map<std::string_view, std::shared_ptr<Bus>> RequestHandler::GetActiveBuses(){
        std::map<std::string_view, std::shared_ptr<Bus>> active_buses;
        for(const auto&[name, bus] : db_.GetBuses()){
            if(!bus.route.empty()){
                active_buses.insert({name, std::make_shared<Bus>(bus)});
            }
        }
        return active_buses;
    }


}//namespace request_handler