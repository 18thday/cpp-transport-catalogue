#include "serialization.h"

#include <fstream>
#include <set>
#include <string>
#include <string_view>

#include <iostream>

#include "json.h"
#include "svg.h"


using namespace std;
using namespace json;

namespace tc {
namespace serialization {

void Serialize(const tc::TransportCatalogue& tc, const std::string& filename){
	tc_serialization::TC TC;
	// bus
	const auto bus_names = tc.GetAllBusNames();
	for (const auto bus_name : bus_names){
		tc_serialization::Bus sbus;
		sbus.set_name(std::string(bus_name));
		sbus.set_is_roundtrip(tc.BusIsRoundtrip(bus_name));
		for (const auto& stop_ptr : tc.GetBusRoute(bus_name)){
			sbus.add_route_stop(stop_ptr->name);
		}
		*TC.add_bus() = sbus;
	}
	// stop
	const auto stop_names = tc.GetAllStopNames();
	for (const auto stop : stop_names){
		const Stop* stop_info = tc.GetStopInfo(stop);
		tc_serialization::Stop sstop;
		sstop.set_name(string(stop_info->name));
		sstop.set_lat(stop_info->coordinates.lat);
		sstop.set_lng(stop_info->coordinates.lng);
//// serialize road_distances

		*TC.add_stop() = sstop;
	}

	ofstream ofs(filename, ios::binary);
	TC.SerializeToOstream(&ofs);
}

void Serialize(const Dict& db, const std::string& filename){
	tc_serialization::TC TC;
    if (db.count("base_requests"s) == 0){
        return;
    }

    Node requests = db.at("base_requests"s);

    for (const auto& request : requests.AsArray()){
    	if (request.AsDict().at("type"s).AsString() == "Stop"s){
    		SerializeStop(TC, request.AsDict());
    	} else if (request.AsDict().at("type"s).AsString() == "Bus"s){
    		SerializeBus(TC, request.AsDict());
    	}
    }
    if (db.count("render_settings"s) != 0){
    	SerializeRenderSettings(TC, db);
   }
    if (db.count("routing_settings"s) != 0){
    	SerializeRoutingSettings(TC, db);
    }



	ofstream ofs(filename, ios::binary);
	TC.SerializeToOstream(&ofs);
}

void SerializeStop(tc_serialization::TC& TC, const json::Dict& request){
	tc_serialization::Stop sstop;
	sstop.set_name(request.at("name"s).AsString());
	sstop.set_lat(request.at("latitude"s).AsDouble());
	sstop.set_lng(request.at("longitude"s).AsDouble());
	if (request.count("road_distances"s) != 0){
		for (const auto& [stop_name, dist] : request.at("road_distances"s).AsDict()){
			sstop.add_to_stop(stop_name);
			sstop.add_dist(dist.AsInt());
		}
	}
	*TC.add_stop() = sstop;
}

void SerializeBus(tc_serialization::TC& TC, const json::Dict& request){
	tc_serialization::Bus sbus;
	sbus.set_name(request.at("name"s).AsString());
	sbus.set_is_roundtrip(request.at("is_roundtrip"s).AsBool());

	for (const auto& stop_node : request.at("stops"s).AsArray()){
		sbus.add_route_stop(stop_node.AsString());
	}

	*TC.add_bus() = sbus;
}

tc_serialization::Color SerializeSVGColor(const Node& from_color){
	tc_serialization::Color color;
    if (from_color.IsArray()){
        const auto& c = from_color.AsArray();
        if (c.size() == 3 || c.size() == 4){
        	color.add_rgb(c[0].AsInt());
        	color.add_rgb(c[1].AsInt());
        	color.add_rgb(c[2].AsInt());
			if (c.size() == 4){
				tc_serialization::Opacity o;
				o.set_opacity(c[3].AsDouble());
				*color.mutable_opacity() = o;
			}
        }
    } else if (from_color.IsString()) {
    	color.set_name(from_color.AsString());
    }
    return std::move(color);
}

void SerializeRenderSettings(tc_serialization::TC& TC, const json::Dict& db){
	tc_serialization::RenderSettings rs;
	const Dict& settings = db.at("render_settings"s).AsDict();

	rs.set_width(settings.at("width"s).AsDouble());
	rs.set_height(settings.at("height"s).AsDouble());
	rs.set_padding(settings.at("padding"s).AsDouble());
	rs.set_line_width(settings.at("line_width"s).AsDouble());
	rs.set_stop_radius(settings.at("stop_radius"s).AsDouble());

	rs.set_bus_label_font_size(settings.at("bus_label_font_size"s).AsInt());
	const auto& offset_b = settings.at("bus_label_offset"s).AsArray();
	tc_serialization::Offset o_b;
	o_b.set_dx(offset_b[0].AsDouble());
	o_b.set_dy(offset_b[1].AsDouble());
	*rs.mutable_bus_label_offset() = o_b;

	rs.set_stop_label_font_size(settings.at("stop_label_font_size"s).AsInt());
	const auto& offset_s = settings.at("stop_label_offset"s).AsArray();
	tc_serialization::Offset o_s;
	o_s.set_dx(offset_s[0].AsDouble());
	o_s.set_dy(offset_s[1].AsDouble());
	*rs.mutable_stop_label_offset() = o_s;


	*rs.mutable_underlayer_color() = SerializeSVGColor(settings.at("underlayer_color"s));
	rs.set_underlayer_width(settings.at("underlayer_width"s).AsDouble());


	for(const auto& color : settings.at("color_palette"s).AsArray()){
		*rs.add_color_palette() = SerializeSVGColor(color);
	}

	*TC.mutable_rs() = rs;
}

void SerializeRoutingSettings(tc_serialization::TC& TC, const json::Dict& db){
	tc_serialization::RoutingSettings routing_s;

    const Dict& settings = db.at("routing_settings"s).AsDict();
    routing_s.set_bus_wait_time(settings.at("bus_wait_time"s).AsInt());
    routing_s.set_bus_velocity(settings.at("bus_velocity"s).AsDouble());

	*TC.mutable_routing_s() = routing_s;
}

//tc::TransportCatalogue Deserialize(const std::string& filename){
//	tc::TransportCatalogue tc;
//	tc_serialization::TC TC;
//
//	ifstream ifs(filename, ios::binary);
//	if (!TC.ParseFromIstream(&ifs)) {
//		return tc;
//	}
//	DeserializeTransportCatalogue(tc, TC);
//
//	return tc;
//}
tc::router::RoutingSettings DeserializeRoutingSettings(const tc_serialization::TC& TC){
	return {TC.routing_s().bus_wait_time(), TC.routing_s().bus_velocity()};
}

tc::renderer::RenderSettings DeserializeRenderSettings(const tc_serialization::TC& TC){
	tc::renderer::RenderSettings rs;


	rs.width = TC.rs().width();
	rs.height = TC.rs().height();
	rs.padding = TC.rs().padding();
	rs.line_width = TC.rs().line_width();
	rs.stop_radius = TC.rs().stop_radius();

	rs.bus_label_font_size = TC.rs().bus_label_font_size();
	rs.bus_label_offset = {TC.rs().bus_label_offset().dx(), TC.rs().bus_label_offset().dy()};

	rs.stop_label_font_size =  TC.rs().stop_label_font_size();
	rs.stop_label_offset = {TC.rs().stop_label_offset().dx(), TC.rs().stop_label_offset().dy()};

	rs.underlayer_color = (TC.rs().has_underlayer_color()) ? DeserializeSVGColor(TC.rs().underlayer_color()) : svg::NoneColor;
	rs.underlayer_width = TC.rs().underlayer_width();

	for (size_t i = 0; i < TC.rs().color_palette_size(); ++i){
		rs.color_palette.push_back(DeserializeSVGColor(TC.rs().color_palette(i)));
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

tc::TransportCatalogue DeserializeTransportCatalogue(const tc_serialization::TC& TC){
	tc::TransportCatalogue tc;

	DeserializeStop(tc, TC);
	DeserializeBus(tc, TC);

	return std::move(tc);
}

void DeserializeStop(tc::TransportCatalogue& tc, const tc_serialization::TC& TC){
    // Add all Stops
	for (size_t i = 0; i < TC.stop_size(); ++i){
		tc.AddStop(TC.stop(i).name(), TC.stop(i).lat(), TC.stop(i).lng());
	}

    // Add distances between Stops
	for (size_t i = 0; i < TC.stop_size(); ++i){
		for (size_t j = 0; j < TC.stop(i).to_stop_size(); ++j){
			 tc.SetDistance(TC.stop(i).name(), TC.stop(i).to_stop(j), TC.stop(i).dist(j));
		}
	}
}
void DeserializeBus(tc::TransportCatalogue& tc, const tc_serialization::TC& TC){
   // Add all Buses
	for (size_t i = 0; i < TC.bus_size(); ++i){
		vector<string_view> stops_for_bus;
		if(TC.bus(i).is_roundtrip()){
			stops_for_bus.reserve(TC.bus(i).route_stop_size());
		} else{
			stops_for_bus.reserve(TC.bus(i).route_stop_size() * 2 - 1);
		}
		for (size_t j = 0; j < TC.bus(i).route_stop_size(); ++j){
			stops_for_bus.push_back(TC.bus(i).route_stop(j));
		}
		if (!TC.bus(i).is_roundtrip()){
//			for (size_t j = TC.bus(i).route_stop_size() - 2; j > 0; --j){
//				stops_for_bus.push_back(TC.bus(i).route_stop(j));
//			}
//			stops_for_bus.push_back(TC.bus(i).route_stop(0));
			size_t size = TC.bus(i).route_stop_size();
			for (size_t j = 1; j < size; ++j){
				stops_for_bus.push_back(TC.bus(i).route_stop(size - 1 - j));
			}
		}

		tc.AddBus(TC.bus(i).name(), stops_for_bus, TC.bus(i).is_roundtrip());
	}
}

} // namespace serialization
} // namespace tc
