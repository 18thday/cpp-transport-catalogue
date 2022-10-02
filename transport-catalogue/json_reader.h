#pragma once

#include "json.h"
#include "json_builder.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "graph.h"
#include "router.h"

namespace tc {
namespace reader {

void ReadJSON(tc::TransportCatalogue& tc, std::istream& input = std::cin);
renderer::RenderSettings ReadRenderSettingsFromJSON(const json::Dict& db);
svg::Color ReadSVGColorFromJSON(const json::Node& from_color);

namespace handler {

struct RoutingSettings{
    int bus_wait_time;
    int bus_velocity;
};

void PerformBaseRequests(tc::TransportCatalogue& tc, const json::Dict& db);
void PerformStatRequests(tc::TransportCatalogue& tc, const json::Dict& db,
                         const renderer::MapRenderer& mr, const graph::DirectedWeightedGraph<double>& tc_graph, const graph::Router<double>& router);

//  BaseRequest Handlers
void AddStop(tc::TransportCatalogue& tc, const json::Dict& request);
void AddStopDistances(tc::TransportCatalogue& tc, const json::Dict& request);
void AddBus(tc::TransportCatalogue& tc, const json::Dict& request);

// Routing handler
RoutingSettings PerformRoutingSettings(const json::Dict& db);
void AddStopsEdgeToGraph(tc::TransportCatalogue& tc, graph::DirectedWeightedGraph<double>& tc_graph, const RoutingSettings& rs);
void AddStopToStopEdgeToGraph(tc::TransportCatalogue& tc, graph::DirectedWeightedGraph<double>& tc_graph, const RoutingSettings& rs);
template <typename InputIt>
void AddBusRouteEdgesToGraph(InputIt begin_range, InputIt end_range,
                             std::string_view bus_name, tc::TransportCatalogue& tc,
                             graph::DirectedWeightedGraph<double>& tc_graph, const RoutingSettings& rs);

//  StatRequest Handlers
void GetStatAnswer(tc::TransportCatalogue& tc, const json::Dict& request, const renderer::MapRenderer& render_settings, json::Builder& bjson,
                const graph::DirectedWeightedGraph<double>& tc_graph, const graph::Router<double>& router);

void RouteStatisticsToDictConvertion(json::Builder& bjson, const tc::RouteStatistics& stat);
void StopRequestToDictConvertion(json::Builder& bjson, const tc::StopRequest& stop);
std::ostringstream& MapRequest(std::ostringstream& str_stream, tc::TransportCatalogue& tc, const renderer::MapRenderer& render_settings);

//  Render
svg::Document RenderMap(const tc::TransportCatalogue& tc, const renderer::MapRenderer& render_settings);

} // namespace handler

} // namespace reader
} // namespace tc


template <typename InputIt>
void tc::reader::handler::AddBusRouteEdgesToGraph(InputIt begin_range, InputIt end_range,
                                                      std::string_view bus_name, tc::TransportCatalogue& tc,
                                                 graph::DirectedWeightedGraph<double>& tc_graph, const RoutingSettings& rs){
    for (auto it_lhs = begin_range; it_lhs != end_range - 1; ++it_lhs){
        double length = 0;
        int span_count = 1;
        auto it_prev_rhs = it_lhs;
        for (auto it_rhs = it_lhs + 1; it_rhs != end_range; it_prev_rhs = it_rhs, ++it_rhs, ++span_count){
            length += tc.GetDistance(*it_prev_rhs, *it_rhs);
//            if(tc.GetPairStopsEdgeId(*it_lhs, *it_rhs)){
//                continue;
//            }
            auto edge_id = tc_graph.AddEdge({tc.GetStopIndex((*it_lhs)->name) + 1, tc.GetStopIndex((*it_rhs)->name), length / 1000 / rs.bus_velocity * 60});
            tc.AddEdgeInfo(edge_id, {EdgeType::BUS, bus_name, span_count});
            tc.AddStopToStopEdge(*it_lhs, *it_rhs ,edge_id);
        }
    }
}
