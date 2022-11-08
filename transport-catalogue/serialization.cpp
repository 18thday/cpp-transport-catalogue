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

void Serialize(const tc::TransportCatalogue& tc, const tc::renderer::RenderSettings& render_set,
               const tc::router::RoutingSettings& routing_set, const std::string& filename){

    tc_serialization::FullModulePack full_pack;
    *full_pack.mutable_transport_catalogue() = std::move(SerializeTransportCatalogue(tc));
    *full_pack.mutable_render_set() = std::move(SerializeRenderSettings(render_set));
    *full_pack.mutable_routing_set() = std::move(SerializeRoutingSettings(routing_set));

    ofstream ofs(filename, ios::binary);
    full_pack.SerializeToOstream(&ofs);
}

tc_serialization::TransportCatalogue SerializeTransportCatalogue(const tc::TransportCatalogue& tc){
    tc_serialization::TransportCatalogue tc_pb;
    SerializeStop(tc_pb, tc);
    SerializeBus(tc_pb, tc);
    return std::move(tc_pb);
}

void SerializeStop(tc_serialization::TransportCatalogue& tc_pb, const tc::TransportCatalogue& tc){
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

void SerializeBus(tc_serialization::TransportCatalogue& tc_pb, const tc::TransportCatalogue& tc){
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

tc_serialization::RenderSettings SerializeRenderSettings(const tc::renderer::RenderSettings& render_set){
    tc_serialization::RenderSettings render_set_pb;

    render_set_pb.set_width(render_set.width);
    render_set_pb.set_height(render_set.height);
    render_set_pb.set_padding(render_set.padding);
    render_set_pb.set_line_width(render_set.line_width);
    render_set_pb.set_stop_radius(render_set.stop_radius);

    render_set_pb.set_bus_label_font_size(render_set.bus_label_font_size);

    tc_serialization::Offset o_b;
    o_b.set_dx(render_set.bus_label_offset.dx);
    o_b.set_dy(render_set.bus_label_offset.dy);
    *render_set_pb.mutable_bus_label_offset() = o_b;

    render_set_pb.set_stop_label_font_size(render_set.stop_label_font_size);

    tc_serialization::Offset o_s;
    o_s.set_dx(render_set.stop_label_offset.dx);
    o_s.set_dy(render_set.stop_label_offset.dy);
    *render_set_pb.mutable_stop_label_offset() = o_s;


    *render_set_pb.mutable_underlayer_color() = SerializeSVGColor(render_set.underlayer_color);
    render_set_pb.set_underlayer_width(render_set.underlayer_width);


    for(const auto& color : render_set.color_palette){
        *render_set_pb.add_color_palette() = SerializeSVGColor(color);
    }

    return std::move(render_set_pb);
}

tc_serialization::RoutingSettings SerializeRoutingSettings(const tc::router::RoutingSettings& routing_set){
    tc_serialization::RoutingSettings routing_set_pb;

    routing_set_pb.set_bus_wait_time(routing_set.bus_wait_time);
    routing_set_pb.set_bus_velocity(routing_set.bus_velocity);

   return std::move(routing_set_pb);
}


//----------- Deserialization ------------

void Deserialize(tc::TransportCatalogue& tc, tc::renderer::RenderSettings& render_set,
                 tc::router::RoutingSettings& routing_set, const std::string& filename){
    tc_serialization::FullModulePack full_pack;

    std::ifstream ifs(filename, std::ios_base::binary);
    if (!full_pack.ParseFromIstream(&ifs)) {
        return;
    }

    tc = tc::serialization::DeserializeTransportCatalogue(full_pack.transport_catalogue());
    render_set = tc::serialization::DeserializeRenderSettings(full_pack.render_set());
    routing_set = tc::serialization::DeserializeRoutingSettings(full_pack.routing_set());
}

tc::router::RoutingSettings DeserializeRoutingSettings(const tc_serialization::RoutingSettings& routing_set_pb){
    return {routing_set_pb.bus_wait_time(), routing_set_pb.bus_velocity()};
}

tc::renderer::RenderSettings DeserializeRenderSettings(const tc_serialization::RenderSettings& render_set_pb){
    tc::renderer::RenderSettings render_set;
    render_set.width = render_set_pb.width();
    render_set.height = render_set_pb.height();
    render_set.padding = render_set_pb.padding();
    render_set.line_width = render_set_pb.line_width();
    render_set.stop_radius = render_set_pb.stop_radius();

    render_set.bus_label_font_size = render_set_pb.bus_label_font_size();
    render_set.bus_label_offset = {render_set_pb.bus_label_offset().dx(),
                           render_set_pb.bus_label_offset().dy()};

    render_set.stop_label_font_size =  render_set_pb.stop_label_font_size();
    render_set.stop_label_offset = {render_set_pb.stop_label_offset().dx(),
                            render_set_pb.stop_label_offset().dy()};

    render_set.underlayer_color = (render_set_pb.has_underlayer_color())
                           ? DeserializeSVGColor(render_set_pb.underlayer_color())
                           : svg::NoneColor;
    render_set.underlayer_width = render_set_pb.underlayer_width();

    for (size_t i = 0; i < render_set_pb.color_palette_size(); ++i){
        render_set.color_palette.push_back(DeserializeSVGColor(render_set_pb.color_palette(i)));
    }

    return render_set;
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

tc::TransportCatalogue DeserializeTransportCatalogue(const tc_serialization::TransportCatalogue& tc_pb){
    tc::TransportCatalogue tc;

    DeserializeStop(tc, tc_pb);
    DeserializeBus(tc, tc_pb);

    return std::move(tc);
}

void DeserializeStop(tc::TransportCatalogue& tc, const tc_serialization::TransportCatalogue& tc_pb){
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
void DeserializeBus(tc::TransportCatalogue& tc, const tc_serialization::TransportCatalogue& tc_pb){
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
