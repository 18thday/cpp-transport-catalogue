#pragma once

#include "geo.h"
#include "svg.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <utility>

namespace tc {
namespace renderer {

inline const double EPSILON = 1e-6;

bool IsZero(double value);

struct Offset{
    double dx;
    double dy;
};

struct RenderSettings{
    double width = 0.0;
    double height = 0.0;

    double padding = 0.0;

    double line_width = 1.0;
    double stop_radius = 0.0;

    int bus_label_font_size = 12;
    Offset bus_label_offset = {0.0, 0.0};

    int stop_label_font_size = 12;
    Offset stop_label_offset = {0.0, 0.0};

    svg::Color underlayer_color;
    double underlayer_width = 0.0;
    std::vector<svg::Color> color_palette;
};

class SphereProjector {
public:
    // points_begin and points_end sets the beginning and end of elements tange in geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding);

    // Projects latitude and longitude into Coordinates inside an SVG-image
    svg::Point operator()(tc::geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

class MapRenderer {
public:
    MapRenderer() = default;
    MapRenderer(RenderSettings render_settings);

    svg::Document RenderMap(const tc::TransportCatalogue& tc) const;

private:
    RenderSettings render_settings_;

    SphereProjector CreateSphereProjectionForBuses(const tc::TransportCatalogue& tc, const std::set<std::string_view>& bus_names) const;

    void AddRouteLinesToSVG(svg::Document& to_svg, const tc::TransportCatalogue& tc, const std::set<std::string_view>& bus_names, const SphereProjector& proj) const;
    void AddBusNamesToSVG(svg::Document& to_svg, const tc::TransportCatalogue& tc, const std::set<std::string_view>& bus_names, const SphereProjector& proj) const;
    void AddStopCircleToSVG(svg::Document& to_svg, const tc::TransportCatalogue& tc, const std::set<std::string_view>& stop_names, const SphereProjector& proj) const;
    void AddStopNamesToSVG(svg::Document& to_svg, const tc::TransportCatalogue& tc, const std::set<std::string_view>& stop_names, const SphereProjector& proj) const;
};

template <typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                                     double max_width, double max_height, double padding)
     : padding_(padding) {
     // If there are no points on the surface of the sphere, there is nothing to calculate
     if (points_begin == points_end) {
         return;
     }

     // Finding points with minimum and maximum longitude
     const auto [left_it, right_it] = std::minmax_element(
         points_begin, points_end,
         [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
     min_lon_ = left_it->lng;
     const double max_lon = right_it->lng;

     // Finding points with minimum and maximum latitude
     const auto [bottom_it, top_it] = std::minmax_element(
         points_begin, points_end,
         [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
     const double min_lat = bottom_it->lat;
     max_lat_ = top_it->lat;

     // Calculate the scaling factor along the x coordinate
     std::optional<double> width_zoom;
     if (!IsZero(max_lon - min_lon_)) {
         width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
     }

     // Calculate the scaling factor along the y coordinate
     std::optional<double> height_zoom;
     if (!IsZero(max_lat_ - min_lat)) {
         height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
     }

     if (width_zoom && height_zoom) {
         // Scaling factors for width and height are non-zero,
    	 // Take the minimum of them
         zoom_coeff_ = std::min(*width_zoom, *height_zoom);
     } else if (width_zoom) {
    	 // Width scaling factor is non-zero, use it
         zoom_coeff_ = *width_zoom;
     } else if (height_zoom) {
    	 // Height scaling factor is non-zero, use it
         zoom_coeff_ = *height_zoom;
     }
 }


} // namespace renderer
} // namespace tc
