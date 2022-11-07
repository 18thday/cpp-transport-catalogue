#include "serialization.h"

#include <fstream>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>

#include <iostream>

#include "json.h"
#include "svg.h"


using namespace std;
using namespace json;

namespace tc {
namespace serialization {

void Serialize(const tc::TransportCatalogue& tc, const tc::renderer::RenderSettings& rs,
               const tc::router::RoutingSettings& routing_s, const std::string& filename){
    tc_serialization::TC tc_pb;

    SerializeStop(tc_pb, tc);
    SerializeBus(tc_pb, tc);
    SerializeRenderSettings(tc_pb, rs);
    SerializeRoutingSettings(tc_pb, routing_s);

    ofstream ofs(filename, ios::binary);
    tc_pb.SerializeToOstream(&ofs);
}

void SerializeStop(tc_serialization::TC& tc_pb, const tc::TransportCatalogue& tc){
    // road_distances
    unordered_map<string_view, vector<pair<string, int>>> stopname_to_road_stop_;
    auto distances_table = tc.GetDistancesTable();
    for (const auto [key, dist] : distances_table){
        stopname_to_road_stop_[key.first->name].push_back(make_pair(string(key.second->name), dist));
    }

    // stop
    const auto stop_names = tc.GetAllStopNames();
    for (const auto stop : stop_names){
        const Stop* stop_info = tc.GetStopInfo(stop);
        tc_serialization::Stop sstop;
        sstop.set_name(string(stop_info->name));
        sstop.set_lat(stop_info->coordinates.lat);
        sstop.set_lng(stop_info->coordinates.lng);
        if (stopname_to_road_stop_.count(sstop.name()) != 0 && stopname_to_road_stop_.at(sstop.name()).size() != 0){
            for (const auto& [to_stop_name, dist] : stopname_to_road_stop_.at(sstop.name())){
                sstop.add_to_stop(string(to_stop_name));
                sstop.add_dist(dist);
            }
        }

        *tc_pb.add_stop() = sstop;
    }
}

void SerializeBus(tc_serialization::TC& tc_pb, const tc::TransportCatalogue& tc){
    const auto bus_names = tc.GetAllBusNames();
    for (const auto bus_name : bus_names){
        tc_serialization::Bus sbus;
        sbus.set_name(std::string(bus_name));
        sbus.set_is_roundtrip(tc.BusIsRoundtrip(bus_name));
        const auto stops = tc.GetBusRoute(bus_name);
        size_t size = tc.BusIsRoundtrip(bus_name) ? stops.size() : stops.size()/2 + 1;
        for (size_t i = 0 ; i < size; ++i){
            sbus.add_route_stop(string(stops[i]->name));
        }
        *tc_pb.add_bus() = sbus;
    }
}

tc_serialization::Color SerializeSVGColor(const svg::Color& from_color){
    tc_serialization::Color color;
    if (holds_alternative<string>(from_color)){
        color.set_name(get<string>(from_color));
    } else if (holds_alternative<svg::Rgb>(from_color)){
        svg::Rgb c = get<svg::Rgb>(from_color);
        color.add_rgb(c.red);
        color.add_rgb(c.green);
        color.add_rgb(c.blue);
    } else if (holds_alternative<svg::Rgba>(from_color)){
        svg::Rgba c = get<svg::Rgba>(from_color);
        color.add_rgb(c.red);
        color.add_rgb(c.green);
        color.add_rgb(c.blue);

        tc_serialization::Opacity o;
        o.set_opacity(c.opacity);
        *color.mutable_opacity() = o;
    }
    return std::move(color);
}

void SerializeRenderSettings(tc_serialization::TC& tc_pb, const tc::renderer::RenderSettings& rs){
    tc_serialization::RenderSettings srs;

    srs.set_width(rs.width);
    srs.set_height(rs.height);
    srs.set_padding(rs.padding);
    srs.set_line_width(rs.line_width);
    srs.set_stop_radius(rs.stop_radius);

    srs.set_bus_label_font_size(rs.bus_label_font_size);

    tc_serialization::Offset o_b;
    o_b.set_dx(rs.bus_label_offset.dx);
    o_b.set_dy(rs.bus_label_offset.dy);
    *srs.mutable_bus_label_offset() = o_b;

    srs.set_stop_label_font_size(rs.stop_label_font_size);

    tc_serialization::Offset o_s;
    o_s.set_dx(rs.stop_label_offset.dx);
    o_s.set_dy(rs.stop_label_offset.dy);
    *srs.mutable_stop_label_offset() = o_s;


    *srs.mutable_underlayer_color() = SerializeSVGColor(rs.underlayer_color);
    srs.set_underlayer_width(rs.underlayer_width);


    for(const auto& color : rs.color_palette){
        *srs.add_color_palette() = SerializeSVGColor(color);
    }

    *tc_pb.mutable_rs() = srs;
}

void SerializeRoutingSettings(tc_serialization::TC& tc_pb, const tc::router::RoutingSettings& routing_s){
    tc_serialization::RoutingSettings srouting_s;

    srouting_s.set_bus_wait_time(routing_s.bus_wait_time);
    srouting_s.set_bus_velocity(routing_s.bus_velocity);

    *tc_pb.mutable_routing_s() = srouting_s;
}

tc::router::RoutingSettings DeserializeRoutingSettings(const tc_serialization::TC& tc_pb){
    return {tc_pb.routing_s().bus_wait_time(), tc_pb.routing_s().bus_velocity()};
}

tc::renderer::RenderSettings DeserializeRenderSettings(const tc_serialization::TC& tc_pb){
    tc::renderer::RenderSettings rs;
    rs.width = tc_pb.rs().width();
    rs.height = tc_pb.rs().height();
    rs.padding = tc_pb.rs().padding();
    rs.line_width = tc_pb.rs().line_width();
    rs.stop_radius = tc_pb.rs().stop_radius();

    rs.bus_label_font_size = tc_pb.rs().bus_label_font_size();
    rs.bus_label_offset = {tc_pb.rs().bus_label_offset().dx(),
                           tc_pb.rs().bus_label_offset().dy()};

    rs.stop_label_font_size =  tc_pb.rs().stop_label_font_size();
    rs.stop_label_offset = {tc_pb.rs().stop_label_offset().dx(),
                            tc_pb.rs().stop_label_offset().dy()};

    rs.underlayer_color = (tc_pb.rs().has_underlayer_color())
                           ? DeserializeSVGColor(tc_pb.rs().underlayer_color())
                           : svg::NoneColor;
    rs.underlayer_width = tc_pb.rs().underlayer_width();

    for (size_t i = 0; i < tc_pb.rs().color_palette_size(); ++i){
        rs.color_palette.push_back(DeserializeSVGColor(tc_pb.rs().color_palette(i)));
    }

    return rs;
}

svg::Color DeserializeSVGColor(tc_serialization::Color color){
    if (color.name() != ""s){
        return color.name();
    }
    if (!color.has_opacity()){
        return svg::Rgb(color.rgb(0), color.rgb(1), color.rgb(2));
    }
    return svg::Rgba(color.rgb(0), color.rgb(1), color.rgb(2), color.opacity().opacity());
}

tc::TransportCatalogue DeserializeTransportCatalogue(const tc_serialization::TC& tc_pb){
    tc::TransportCatalogue tc;

    DeserializeStop(tc, tc_pb);
    DeserializeBus(tc, tc_pb);

    return std::move(tc);
}

void DeserializeStop(tc::TransportCatalogue& tc, const tc_serialization::TC& tc_pb){
    // Add all Stops
    for (size_t i = 0; i < tc_pb.stop_size(); ++i){
        tc.AddStop(tc_pb.stop(i).name(), tc_pb.stop(i).lat(), tc_pb.stop(i).lng());
    }

    // Add distances between Stops
    for (size_t i = 0; i < tc_pb.stop_size(); ++i){
        for (size_t j = 0; j < tc_pb.stop(i).to_stop_size(); ++j){
             tc.SetDistance(tc_pb.stop(i).name(), tc_pb.stop(i).to_stop(j), tc_pb.stop(i).dist(j));
        }
    }
}
void DeserializeBus(tc::TransportCatalogue& tc, const tc_serialization::TC& tc_pb){
   // Add all Buses
    for (size_t i = 0; i < tc_pb.bus_size(); ++i){
        vector<string_view> stops_for_bus;
        if(tc_pb.bus(i).is_roundtrip()){
            stops_for_bus.reserve(tc_pb.bus(i).route_stop_size());
        } else{
            stops_for_bus.reserve(tc_pb.bus(i).route_stop_size() * 2 - 1);
        }
        for (size_t j = 0; j < tc_pb.bus(i).route_stop_size(); ++j){
            stops_for_bus.push_back(tc_pb.bus(i).route_stop(j));
        }
        if (!tc_pb.bus(i).is_roundtrip()){
            size_t size = tc_pb.bus(i).route_stop_size();
            for (size_t j = 1; j < size; ++j){
                stops_for_bus.push_back(tc_pb.bus(i).route_stop(size - 1 - j));
            }
        }

        tc.AddBus(tc_pb.bus(i).name(), stops_for_bus, tc_pb.bus(i).is_roundtrip());
    }
}

} // namespace serialization
} // namespace tc
