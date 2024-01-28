#include "map_renderer.h"

namespace Renderer
{
	MapRenderer::MapRenderer(const RenderSettings& settings)
		: settings_(settings)
	{ }

	const RenderSettings& MapRenderer::GetSettings() const
	{
		return settings_;
	}

	void MapRenderer::SetSphereProjector(const std::vector<geo::Coordinates>& coords)
	{
		projector_ = SphereProjector(coords.cbegin(), coords.cend(), settings_.width, settings_.height, settings_.padding);
	}

	svg::Point MapRenderer::Project(const geo::Coordinates& coords) const
	{
		return projector_(coords);
	}

	svg::Polyline MapRenderer::CreateRouteLine(const svg::Color& color) const
	{
		using namespace std::string_literals;
		svg::Polyline line;
		line.SetStrokeColor(color);
		line.SetFillColor("none"s).SetStrokeWidth(settings_.line_width);
		return line.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	}

	svg::Circle MapRenderer::CreateStopCircle(const svg::Point& point) const
	{
		svg::Circle circle;
		return circle.SetCenter(point).SetFillColor("white").SetRadius(settings_.stop_radius);
	}

	svg::Text MapRenderer::CreateRouteName(const std::string& name, const svg::Point& point, const svg::Color& color) const
	{
		svg::Text label;
		label.SetFillColor(color);
		label.SetPosition(point).SetOffset({ settings_.bus_label_offset.first, settings_.bus_label_offset.second });
		label.SetFontSize(settings_.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold");
		label.SetData(name);


		return label;
	}
	svg::Text MapRenderer::CreateMountForLabel(const svg::Text& label) const
	{
		svg::Text mount(label);
		mount.SetFillColor(settings_.underlayer_color);
		mount.SetStrokeColor(settings_.underlayer_color).SetStrokeWidth(settings_.underlayer_width);
		mount.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		return mount;
	}

	svg::Text MapRenderer::CreateStopName(const std::string& name, const svg::Point& point) const
	{
		using namespace std::string_literals;
		svg::Text label;
		label.SetPosition(point).SetOffset({ settings_.stop_label_offset.first, settings_.stop_label_offset.second });
		label.SetFontSize(settings_.stop_label_font_size).SetFontFamily("Verdana");
		label.SetData(name).SetFillColor("black");
		return label;
	}
}
