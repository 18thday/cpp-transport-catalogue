#pragma once

#include "transport_catalogue.h"
#include "router.h"
#include "graph.h"

#include <functional>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <utility>

namespace tc{
namespace router{

struct RoutingSettings{
    int bus_wait_time;
    double bus_velocity;
};

struct EdgeInfo{
    EdgeType type;
    std::string_view name;
    std::optional<int> span_count = std::nullopt;
};

using RouteInfo = graph::Router<double>::RouteInfo;

class Router{
private:
    std::unordered_map<const Stop*, size_t> stopptr_to_graph_;
    std::unordered_map<size_t, EdgeInfo> graph_edge_to_info_;

    struct StopToStopHasher{
        size_t operator() (const std::pair<Stop*, Stop*>& pairstops) const;
    };
    std::unordered_map<std::pair<Stop*, Stop*>, size_t, StopToStopHasher> pairstops_to_edge_id_;

public:
    Router(RoutingSettings setting, const TransportCatalogue& tc);

    std::optional<RouteInfo> FindRoute(std::string_view stop_from, std::string_view stop_to) const;

    const graph::Edge<double>& GetEdge(size_t edge_id) const;
    const EdgeInfo& GetEdgeInfo(size_t edge_id) const;

private:
    RoutingSettings settings_;
    const TransportCatalogue& tc_;
    graph::DirectedWeightedGraph<double> tc_graph_;
    graph::Router<double> router_;

    const graph::DirectedWeightedGraph<double>& ReturnInitializedGraph();
    void CreateGraph();
    void AddStopsEdgeToGraph();
    void AddStopToStopEdgeToGraph();
    template <typename InputIt>
    void AddBusRouteEdgesToGraph(InputIt begin_range, InputIt end_range, std::string_view bus_name);

    void AddEdgeInfo(size_t edge_id, EdgeInfo edge_info);
    void AddStopToStopEdge(Stop* from, Stop* to, size_t edge_id);

    size_t GetStopIndex(std::string_view stop_name) const;
    std::optional<size_t> GetPairStopsEdgeId(Stop* from, Stop* to);
};

template <typename InputIt>
void Router::AddBusRouteEdgesToGraph(InputIt begin_range, InputIt end_range, std::string_view bus_name){
    for (auto it_lhs = begin_range; it_lhs != end_range - 1; ++it_lhs){
        double length = 0;
        int span_count = 1;
        auto it_prev_rhs = it_lhs;
        for (auto it_rhs = it_lhs + 1; it_rhs != end_range; it_prev_rhs = it_rhs, ++it_rhs, ++span_count){
            length += tc_.GetDistance(*it_prev_rhs, *it_rhs);
            auto edge_id = tc_graph_.AddEdge({GetStopIndex((*it_lhs)->name) + 1, GetStopIndex((*it_rhs)->name), length / 1000 / settings_.bus_velocity * 60});
            AddEdgeInfo(edge_id, {EdgeType::BUS, bus_name, span_count});
            AddStopToStopEdge(*it_lhs, *it_rhs ,edge_id);
        }
    }
}

} // namespace router
} // namespace tc






