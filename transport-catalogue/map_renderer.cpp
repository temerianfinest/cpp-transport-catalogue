#include "map_renderer.h"

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Polyline MapRenderer::RenderRoute(std::shared_ptr<Bus>& bus, const svg::Color& color) {
    svg::Polyline route;
    route.SetFillColor(svg::NoneColor).SetStrokeColor(color).SetStrokeWidth(renderer_settings_.line_width).
        SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    for(auto it = bus->route.begin(); it != bus->route.end(); ++it){
        route.AddPoint(canvas_({(*it)->latitude, (*it)->longitude}));
    }
    if(!bus->is_circle){
        for(auto it = bus->route.rbegin() + 1; it != bus->route.rend(); ++it){
            route.AddPoint(canvas_({(*it)->latitude, (*it)->longitude}));
        }
    }
    return route;
}

std::vector<geo::Coordinates> MapRenderer::GetStopsCoordinates(const std::map<std::string_view, std::shared_ptr<Bus>>& buses){
    std::vector<geo::Coordinates> stop_coordinates;
    for(const auto& [bus_name, bus] : buses){
        for(const auto& stop : bus->route){
            if(!stops_to_render_.count(stop->stop_name)) {
                stop_coordinates.push_back({stop->latitude, stop->longitude});
                stops_to_render_.insert({stop->stop_name, stop});
            }
        }
    }
    return stop_coordinates;
}

svg::Color MapRenderer::ColorSelector(uint32_t index) {
    return svg::Color{renderer_settings_.color_palette[index % renderer_settings_.color_palette.size()]};
}

std::pair<svg::Text, svg::Text> MapRenderer::RenderSingleRouteName(std::shared_ptr<Bus> &bus, svg::Point text_coords, const svg::Color &color) {
    svg::Text bus_label_underlayer = svg::Text()
            .SetPosition(text_coords)
            .SetOffset(renderer_settings_.bus_label_offset)
            .SetFillColor(renderer_settings_.underlayer_color)
            .SetStrokeColor(renderer_settings_.underlayer_color)
            .SetFontSize(renderer_settings_.bus_label_font_size)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetStrokeWidth(renderer_settings_.underlayer_width)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetData({bus->bus_name.data(), bus->bus_name.size()});
    svg::Text bus_label = svg::Text()
            .SetPosition(text_coords)
            .SetOffset(renderer_settings_.bus_label_offset)
            .SetFillColor(color)
            .SetFontSize(renderer_settings_.bus_label_font_size)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetData({bus->bus_name.data(), bus->bus_name.size()});
    return {bus_label_underlayer, bus_label};
}

svg::Document MapRenderer::RenderMap() {
    svg::Document doc;
    uint32_t index = 0;
    for(auto [bus_name, bus] : routes_to_render_){
        doc.Add(RenderRoute(bus, ColorSelector(index)));
        ++index;
    }
    index = 0;
    for(auto [bus_name, bus] : routes_to_render_){
        svg::Point label_coords = canvas_({bus->route.front()->latitude, bus->route.front()->longitude});
        doc.Add((RenderSingleRouteName(bus, label_coords, ColorSelector(index)).first));
        doc.Add((RenderSingleRouteName(bus, label_coords, ColorSelector(index)).second));
        if(!bus->is_circle && bus->route.front() != bus->route.back()){
            svg::Point end_label_coords = canvas_({bus->route.back()->latitude, bus->route.back()->longitude});
            doc.Add((RenderSingleRouteName(bus, end_label_coords, ColorSelector(index)).first));
            doc.Add((RenderSingleRouteName(bus, end_label_coords, ColorSelector(index)).second));
        }
        ++index;
    }
    for(const auto& [stop_name, stop] :stops_to_render_){
            doc.Add(RenderStopCircle(stop, "white"));
    }
    for(const auto& [stop_name, stop] :stops_to_render_){
        doc.Add(RenderStopName(stop, "black").first);
        doc.Add(RenderStopName(stop, "black").second);
    }
    return doc;
}

svg::Circle MapRenderer::RenderStopCircle(const Stop* stop, const svg::Color &color) {
    svg::Point circle_coords = canvas_({stop->latitude, stop->longitude});
    svg::Circle circle = svg::Circle()
            .SetCenter(circle_coords)
            .SetRadius(renderer_settings_.stop_radius)
            .SetFillColor(color);
    return circle;
}

std::pair<svg::Text, svg::Text>
MapRenderer::RenderStopName(const Stop* stop, const svg::Color &color) {
    svg::Point text_coords = canvas_({stop->latitude, stop->longitude});
    svg::Text stop_label_underlayer = svg::Text()
            .SetPosition(text_coords)
            .SetOffset(renderer_settings_.stop_label_offset)
            .SetFillColor(renderer_settings_.underlayer_color)
            .SetStrokeColor(renderer_settings_.underlayer_color)
            .SetFontSize(renderer_settings_.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetStrokeWidth(renderer_settings_.underlayer_width)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetData({stop->stop_name.data(), stop->stop_name.size()});
    svg::Text stop_label = svg::Text()
            .SetPosition(text_coords)
            .SetOffset(renderer_settings_.stop_label_offset)
            .SetFillColor(color)
            .SetFontSize(renderer_settings_.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetData({stop->stop_name.data(), stop->stop_name.size()});
    return {stop_label_underlayer, stop_label};
}