#include "json_reader.h"
#include "request_handler.h"
#include "svg.h"
#include "map_renderer.h"

#include <iostream>
#include <string>
#include <string_view>
#include <sstream>

using namespace std;
using namespace json;
using namespace tc::renderer;

namespace tc {
namespace reader {

void ReadJSON(tc::TransportCatalogue& tc, istream& input){
    Node main_node = Load(input).GetRoot();
    if (main_node.IsMap()){
//        Dict db = main_node.AsMap();

        handler::PerformBaseRequests(tc, main_node.AsMap());
        MapRenderer map_renderer(ReadRenderSettingsFromJSON(main_node.AsMap()));
        handler::PerformStatRequests(tc, main_node.AsMap(), map_renderer);
    }
}

RenderSettings ReadRenderSettingsFromJSON(const Dict& db){
    if (db.count("render_settings"s) == 0){
        return {};
    }
    RenderSettings render_setting;

    Dict setings = db.at("render_settings"s).AsMap();

    render_setting.width = setings.at("width"s).AsDouble();
    render_setting.height = setings.at("height"s).AsDouble();
    render_setting.padding = setings.at("padding"s).AsDouble();
    render_setting.line_width = setings.at("line_width"s).AsDouble();
    render_setting.stop_radius = setings.at("stop_radius"s).AsDouble();

    render_setting.bus_label_font_size = setings.at("bus_label_font_size"s).AsInt();
    const auto& offset_b = setings.at("bus_label_offset"s).AsArray();
    render_setting.bus_label_offset = {offset_b[0].AsDouble(), offset_b[1].AsDouble()};

    render_setting.stop_label_font_size = setings.at("stop_label_font_size"s).AsInt();
    const auto& offset_s = setings.at("stop_label_offset"s).AsArray();
    render_setting.stop_label_offset = {offset_s[0].AsDouble(), offset_s[1].AsDouble()};

    render_setting.underlayer_color = ReadSVGColorFromJSON(setings.at("underlayer_color"s));
    render_setting.underlayer_width = setings.at("underlayer_width"s).AsDouble();

    for(const auto& color : setings.at("color_palette"s).AsArray()){
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

namespace handler {
// Base request
void PerformBaseRequests(tc::TransportCatalogue& tc, const Dict& db){
    if (db.count("base_requests"s) == 0){
        return;
    }

    Node requests = db.at("base_requests"s);
    // Add all Stops
    for (const auto& request : requests.AsArray()){
        AddStop(tc, request.AsMap());
    }

    // Add distances between Stops
    for (const auto& request : requests.AsArray()){
        AddStopDistances(tc, request.AsMap());
    }

    // Add all Buses
    for (const auto& request : requests.AsArray()){
        AddBus(tc, request.AsMap());
    }
}

void AddStop(tc::TransportCatalogue& tc, const Dict& request){
    if (request.at("type"s).AsString() == "Stop"s){
        tc.AddStop(request.at("name"s).AsString(), request.at("latitude"s).AsDouble(), request.at("longitude"s).AsDouble());
    }
}

void AddStopDistances(tc::TransportCatalogue& tc, const Dict& request){
    if ( (request.at("type"s).AsString() == "Stop"s) && (request.count("road_distances"s) != 0) ){
        for (const auto& [stop_name, dist] : request.at("road_distances"s).AsMap()){
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

// Stat Request
void PerformStatRequests(tc::TransportCatalogue& tc, const Dict& db, const renderer::MapRenderer& mr){
    if (db.count("stat_requests"s) == 0){
        return;
    }

    Node requests = db.at("stat_requests"s);

    Array stat;
    stat.reserve(requests.AsArray().size());

    for (const auto& request : requests.AsArray()){
        stat.push_back(GetStatAnswer(tc, request.AsMap(), mr));
    }

    json::PrintNode(stat, std::cout);
}



Dict GetStatAnswer(tc::TransportCatalogue& tc, const Dict& request, const renderer::MapRenderer& mr){
    Dict result;
    result["request_id"s] = request.at("id").AsInt();
    if (request.at("type").AsString() == "Bus"s){
        tc::RouteStatistics stat = tc.GetStatistics(request.at("name").AsString());
        if (stat.stops == 0){
            result["error_message"s] = "not found"s;
            return result;
        }
        RouteStatisticsToDictConvertion(result, stat);
    } else if (request.at("type").AsString() == "Stop"s){
        tc::StopRequest stop = tc.GetBusForStop(request.at("name").AsString());
        if (!stop.have_stop){
            result["error_message"s] = "not found"s;
            return result;
        }
        StopRequestToDictConvertion(result, stop);
    } else if (request.at("type").AsString() == "Map"s){
        std::ostringstream str_stream;
        result["map"s] = MapRequest(str_stream, tc, mr).str();
    }
    return result;
}

void RouteStatisticsToDictConvertion(Dict& dict, const tc::RouteStatistics& stat){
    dict["stop_count"] = Node(static_cast<int>(stat.stops));
    dict["unique_stop_count"] = Node(static_cast<int>(stat.unique_stops));
    dict["route_length"] = Node(stat.route_length);
    dict["curvature"] = Node(stat.curvature);
}

void StopRequestToDictConvertion(Dict& dict, const tc::StopRequest& stop){
    Array buses;
    for (const auto& bus : stop.all_buses){
        buses.push_back(Node(string(bus)));
    }
    dict["buses"] = buses;
}

std::ostringstream& MapRequest(std::ostringstream& str_stream, tc::TransportCatalogue& tc, const renderer::MapRenderer& mr){
    svg::Document svg_doc = mr.RenderMap(tc);
    svg_doc.Render(str_stream);
    return str_stream;
}

} // namespace handler

} // namespace reader
} // namespace tc
