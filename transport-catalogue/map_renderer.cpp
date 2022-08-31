#include "map_renderer.h"

namespace tc{
namespace renderer{

using namespace std;
using namespace svg;

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Point SphereProjector::operator()(tc::geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}


MapRenderer::MapRenderer(RenderSettings render_settings)
    : render_settings_(render_settings) {
}

svg::Document MapRenderer::RenderMap(const tc::TransportCatalogue& tc) const{
    svg::Document result;
    auto bus_names = tc.GetAllBusNames();

    auto proj = CreateSphereProjectionForBuses(tc, bus_names);
    // Lines
    AddRouteLinesToSVG(result, tc, bus_names, proj);
    // Text
    AddBusNamesToSVG(result, tc, bus_names, proj);
    // Circle
    auto stop_names = tc.GetAllStopNames();
    AddStopCircleToSVG(result, tc, stop_names, proj);
    //Stop names
    AddStopNamesToSVG(result, tc, stop_names, proj);

    return result;
}

SphereProjector MapRenderer::CreateSphereProjectionForBuses(const tc::TransportCatalogue& tc, const std::set<std::string_view>& bus_names) const{
    vector<tc::geo::Coordinates> all_coordinates;
    for (const auto bus_name : bus_names){
        for (const auto stop : tc.GetBusRoute(bus_name)){
            all_coordinates.push_back(stop->coordinates);
        }
    }
    return SphereProjector(all_coordinates.begin(), all_coordinates.end(),
            render_settings_.width, render_settings_.height, render_settings_.padding);
}

void MapRenderer::AddRouteLinesToSVG(svg::Document& to_svg, const tc::TransportCatalogue& tc, const std::set<std::string_view>& bus_names, const SphereProjector& proj) const{
    int i = 0;
    for (auto it = bus_names.begin(); it != bus_names.end(); ++it, ++i){
        svg::Polyline polyline;
        for (const auto stop : tc.GetBusRoute(*it)){
            polyline.AddPoint(proj(stop->coordinates));
        }
        polyline.SetFillColor("none"s)
                .SetStrokeColor(render_settings_.color_palette[i % render_settings_.color_palette.size()])
                .SetStrokeWidth(render_settings_.line_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        to_svg.Add(polyline);
    }
}

void MapRenderer::AddBusNamesToSVG(svg::Document& to_svg, const tc::TransportCatalogue& tc, const std::set<std::string_view>& bus_names, const SphereProjector& proj) const{
    int i = 0;
    for (auto it = bus_names.begin(); it != bus_names.end(); ++it, ++i){
        auto root = tc.GetBusRoute(*it);
        if (root.empty()){
            continue;
        }
        svg::Text under_text;
        under_text.SetPosition(proj(root[0]->coordinates))
                  .SetOffset({render_settings_.bus_label_offset.dx, render_settings_.bus_label_offset.dy})
                  .SetFontSize(render_settings_.bus_label_font_size)
                  .SetFontFamily("Verdana"s)
                  .SetFontWeight("bold"s)
                  .SetData(string(*it));
        svg::Text text = under_text;

        under_text.SetFillColor(render_settings_.underlayer_color).SetStrokeColor(render_settings_.underlayer_color)
                  .SetStrokeWidth(render_settings_.underlayer_width)
                  .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                  .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        text.SetFillColor(render_settings_.color_palette[i % render_settings_.color_palette.size()]);
        to_svg.Add(under_text);
        to_svg.Add(text);
        if (!tc.BusIsRoundtrip(*it) && root[root.size()/2] != root[0]){
            under_text.SetPosition(proj(root[root.size()/2]->coordinates));
            text.SetPosition(proj(root[root.size()/2]->coordinates));
            to_svg.Add(under_text);
            to_svg.Add(text);
        }
    }
}

void MapRenderer::AddStopCircleToSVG(svg::Document& to_svg, const tc::TransportCatalogue& tc, const std::set<std::string_view>& stop_names, const SphereProjector& proj) const{
    for (const auto stop_name : stop_names){
        if (!tc.StopHaveBus(stop_name)){
            continue;
        }
        svg::Circle stop_circle;
        auto stop = tc.GetStopInfo(stop_name);
        stop_circle.SetCenter(proj(stop->coordinates))
                   .SetRadius(render_settings_.stop_radius)
                   .SetFillColor("white"s);

        to_svg.Add(stop_circle);
    }
}

void MapRenderer::AddStopNamesToSVG(svg::Document& to_svg, const tc::TransportCatalogue& tc, const std::set<std::string_view>& stop_names, const SphereProjector& proj) const{
    for (const auto stop_name : stop_names){
        if (!tc.StopHaveBus(stop_name)){
            continue;
        }

        auto stop = tc.GetStopInfo(stop_name);
        svg::Text under_text;
        under_text.SetPosition(proj(stop->coordinates))
                  .SetOffset({render_settings_.stop_label_offset.dx, render_settings_.stop_label_offset.dy})
                  .SetFontSize(render_settings_.stop_label_font_size)
                  .SetFontFamily("Verdana"s)
                  .SetData(string(stop_name));

        svg::Text text = under_text;
        under_text.SetFillColor(render_settings_.underlayer_color).SetStrokeColor(render_settings_.underlayer_color)
                  .SetStrokeWidth(render_settings_.underlayer_width)
                  .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                  .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        text.SetFillColor("black"s);

        to_svg.Add(under_text);
        to_svg.Add(text);
    }
}

} // namespace renderer
} // namespace tc
