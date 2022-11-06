#include "json_reader.h"
#include "request_handler.h"
#include "svg.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "graph.h"
#include "router.h"
#include "transport_router.h"
#include "serialization.h"


#include <iostream>
#include <string>
#include <string_view>
#include <sstream>
#include <fstream>

using namespace std;
using namespace json;
using namespace tc::renderer;

using TC_Graph = graph::DirectedWeightedGraph<double>;

namespace tc {
namespace reader {

void ReadJSON(tc::TransportCatalogue& tc, istream& input){
    Node main_node = Load(input).GetRoot();
    if (main_node.IsDict()){

        handler::PerformBaseRequests(tc, main_node.AsDict());

        tc::router::Router router(handler::PerformRoutingSettings(main_node.AsDict()), tc);

        MapRenderer map_renderer(ReadRenderSettingsFromJSON(main_node.AsDict()));

        std::string filename = ReadSerializationSettingsFromJSON(main_node.AsDict());
//        tc::serialization::Serialize(tc, filename);
//        tc::serialization::Serialize(main_node.AsDict(), filename);

        handler::PerformStatRequests(tc, main_node.AsDict(), map_renderer, router);
    }
}

void MakeBaseFromJSON(tc::TransportCatalogue& tc, istream& input){
    Node main_node = Load(input).GetRoot();
    if (main_node.IsDict()){

        handler::PerformBaseRequests(tc, main_node.AsDict());
        std::string filename = ReadSerializationSettingsFromJSON(main_node.AsDict());

//        tc::serialization::Serialize(tc, filename);
        tc::serialization::Serialize(main_node.AsDict(), filename);
    }
}
void ProcessRequestFromJSON(istream& input){
    Node main_node = Load(input).GetRoot();
    if (main_node.IsDict()){

        std::string filename = ReadSerializationSettingsFromJSON(main_node.AsDict());

    	tc_serialization::TC TC;

    	std::ifstream ifs(filename, std::ios_base::binary);
    	if (!TC.ParseFromIstream(&ifs)) {
    		return;
    	}

        tc::TransportCatalogue tc = tc::serialization::DeserializeTransportCatalogue(TC);
        MapRenderer map_renderer(tc::serialization::DeserializeRenderSettings(TC));
        tc::router::Router router(tc::serialization::DeserializeRoutingSettings(TC), tc);

        handler::PerformStatRequests(tc, main_node.AsDict(), map_renderer, router);
//        handler::PerformStatRequests(tc, main_node.AsDict(), map_renderer);
    }
}

RenderSettings ReadRenderSettingsFromJSON(const Dict& db){
    if (db.count("render_settings"s) == 0){
        return {};
    }
    RenderSettings render_setting;

    Dict settings = db.at("render_settings"s).AsDict();

    render_setting.width = settings.at("width"s).AsDouble();
    render_setting.height = settings.at("height"s).AsDouble();
    render_setting.padding = settings.at("padding"s).AsDouble();
    render_setting.line_width = settings.at("line_width"s).AsDouble();
    render_setting.stop_radius = settings.at("stop_radius"s).AsDouble();

    render_setting.bus_label_font_size = settings.at("bus_label_font_size"s).AsInt();
    const auto& offset_b = settings.at("bus_label_offset"s).AsArray();
    render_setting.bus_label_offset = {offset_b[0].AsDouble(), offset_b[1].AsDouble()};

    render_setting.stop_label_font_size = settings.at("stop_label_font_size"s).AsInt();
    const auto& offset_s = settings.at("stop_label_offset"s).AsArray();
    render_setting.stop_label_offset = {offset_s[0].AsDouble(), offset_s[1].AsDouble()};

    render_setting.underlayer_color = ReadSVGColorFromJSON(settings.at("underlayer_color"s));
    render_setting.underlayer_width = settings.at("underlayer_width"s).AsDouble();

    for(const auto& color : settings.at("color_palette"s).AsArray()){
        render_setting.color_palette.push_back(ReadSVGColorFromJSON(color));
    }
    return render_setting;
}

svg::Color ReadSVGColorFromJSON(const Node& from_color){
    if (from_color.IsArray()){
        const auto& color = from_color.AsArray();
        if (color.size() == 3){
            return svg::Rgb(color[0].AsInt(), color[1].AsInt(), color[2].AsInt());
        }
        if (color.size() == 4){
            return svg::Rgba(color[0].AsInt(), color[1].AsInt(), color[2].AsInt(), color[3].AsDouble());
        }
    }
    return from_color.AsString();
}

std::string ReadSerializationSettingsFromJSON(const Dict& db){
	if (db.count("serialization_settings"s) == 0){
		return {};
	}
	Dict settings = db.at("serialization_settings"s).AsDict();
	return settings.at("file"s).AsString();
}

namespace handler {

// Base request
void PerformBaseRequests(tc::TransportCatalogue& tc, const Dict& db){
    if (db.count("base_requests"s) == 0){
        return;
    }

    Node requests = db.at("base_requests"s);
    // Add all Stops
    for (const auto& request : requests.AsArray()){
        AddStop(tc, request.AsDict());
    }

    // Add distances between Stops
    for (const auto& request : requests.AsArray()){
        AddStopDistances(tc, request.AsDict());
    }

    // Add all Buses
    for (const auto& request : requests.AsArray()){
        AddBus(tc, request.AsDict());
    }
}

void AddStop(tc::TransportCatalogue& tc, const Dict& request){
    if (request.at("type"s).AsString() == "Stop"s){
        tc.AddStop(request.at("name"s).AsString(), request.at("latitude"s).AsDouble(), request.at("longitude"s).AsDouble());
    }
}

void AddStopDistances(tc::TransportCatalogue& tc, const Dict& request){
    if ( (request.at("type"s).AsString() == "Stop"s) && (request.count("road_distances"s) != 0) ){
        for (const auto& [stop_name, dist] : request.at("road_distances"s).AsDict()){
            tc.SetDistance(request.at("name"s).AsString(), stop_name, dist.AsInt());
        }
    }
}

void AddBus(tc::TransportCatalogue& tc, const Dict& request){
    if (request.at("type"s).AsString() != "Bus"s){
        return;
    }
    const auto& stops = request.at("stops"s).AsArray();
    bool is_roundtrip = request.at("is_roundtrip"s).AsBool();

    vector<string_view> stops_for_bus;
    if(is_roundtrip){
        stops_for_bus.reserve(stops.size());
    } else{
        stops_for_bus.reserve(stops.size() * 2 - 1);
    }

    for (const auto& stop_node : stops){
        stops_for_bus.push_back(stop_node.AsString());
    }

    if (!is_roundtrip){
        for(auto it = stops.rbegin() + 1; it < stops.rend(); ++it){
            stops_for_bus.push_back((*it).AsString());
        }
    }

    tc.AddBus(request.at("name"s).AsString(), stops_for_bus, is_roundtrip);
}

//Routing handler
tc::router::RoutingSettings PerformRoutingSettings(const Dict& db){
    if (db.count("routing_settings"s) == 0){
        return {};
    }

    Dict settings = db.at("routing_settings"s).AsDict();
    return {settings.at("bus_wait_time"s).AsInt(), settings.at("bus_velocity"s).AsDouble()};
}

// Stat Request
void PerformStatRequests(tc::TransportCatalogue& tc, const Dict& db, const renderer::MapRenderer& mr, const tc::router::Router& router){

    if (db.count("stat_requests"s) == 0){
        return;
    }

    Node requests = db.at("stat_requests"s);

    Array stat;
    stat.reserve(requests.AsArray().size());

    auto builderJSON = json::Builder{};
    builderJSON.StartArray();
    for (const auto& request : requests.AsArray()){
        GetStatAnswer(tc, request.AsDict(), mr, builderJSON, router);
    }

    json::Print(
        json::Document{
            builderJSON
            .EndArray()
            .Build()
        },
        cout
      );
      cout << endl;
}

void GetStatAnswer(tc::TransportCatalogue& tc, const Dict& request, const renderer::MapRenderer& mr, Builder& bjson, const tc::router::Router& router){
//    cerr << "GetStatAnswer" << endl;
    bjson.StartDict().Key("request_id"s).Value(request.at("id").AsInt());

    if (request.at("type").AsString() == "Bus"s){
        tc::RouteStatistics stat = tc.GetStatistics(request.at("name").AsString());
        if (stat.stops == 0){
            bjson.Key("error_message"s).Value("not found"s).EndDict();
            return;
        }
        RouteStatisticsToDictConvertion(bjson, stat);

    } else if (request.at("type").AsString() == "Stop"s){
        tc::StopRequest stop = tc.GetBusForStop(request.at("name").AsString());
        if (!stop.have_stop){
            bjson.Key("error_message"s).Value("not found"s).EndDict();
            return;
        }
        StopRequestToDictConvertion(bjson, stop);

    } else if (request.at("type").AsString() == "Map"s){
        std::ostringstream str_stream;
        bjson.Key("map"s).Value(MapRequest(str_stream, tc, mr).str());

    } else if (request.at("type").AsString() == "Route"s){
        string stop_from = request.at("from"s).AsString();
        string stop_to = request.at("to"s).AsString();

        auto route = router.FindRoute(stop_from, stop_to);

        if (!route){
            bjson.Key("error_message"s).Value("not found"s).EndDict();
            return;
        }
        bjson.Key("items"s).StartArray();
        for (const auto& edge_id : route.value().edges){
            tc::router::EdgeInfo edge_info = router.GetEdgeInfo(edge_id);
            bjson.StartDict();
            if (edge_info.type == EdgeType::WAIT){
                bjson.Key("type"s).Value("Wait"s);
                bjson.Key("stop_name"s).Value(string(edge_info.name));
            } else if (edge_info.type == EdgeType::BUS){
                bjson.Key("type"s).Value("Bus"s);
                bjson.Key("bus"s).Value(string(edge_info.name));
                bjson.Key("span_count"s).Value(edge_info.span_count.value());
            }
            bjson.Key("time"s).Value((router.GetEdge(edge_id)).weight);
            bjson.EndDict();
        }
        bjson.EndArray();

        bjson.Key("total_time"s).Value(route.value().weight);
    }
    bjson.EndDict();
}

void RouteStatisticsToDictConvertion(Builder& bjson, const tc::RouteStatistics& stat){
    bjson.Key("stop_count"s).Value(static_cast<int>(stat.stops));
    bjson.Key("unique_stop_count"s).Value(static_cast<int>(stat.unique_stops));
    bjson.Key("route_length"s).Value(stat.route_length);
    bjson.Key("curvature"s).Value(stat.curvature);
}

void StopRequestToDictConvertion(Builder& bjson, const tc::StopRequest& stop){
    bjson.Key("buses"s).StartArray();
    for (const auto& bus : stop.all_buses){
        bjson.Value(string(bus));
    }
    bjson.EndArray();
}

std::ostringstream& MapRequest(std::ostringstream& str_stream, tc::TransportCatalogue& tc, const renderer::MapRenderer& mr){
    svg::Document svg_doc = mr.RenderMap(tc);
    svg_doc.Render(str_stream);
    return str_stream;
}

} // namespace handler

} // namespace reader
} // namespace tc
