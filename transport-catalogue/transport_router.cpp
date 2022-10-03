#include "transport_router.h"

namespace tc{
namespace router{

Router::Router(RoutingSettings setting, const TransportCatalogue& tc)
: settings_(setting),
  tc_(tc),
  tc_graph_(tc.GetStopCount() * 2),
  router_(ReturnInitializedGraph()){
}

std::optional<RouteInfo> Router::FindRoute(std::string_view stop_from, std::string_view stop_to) const{
    return router_.BuildRoute(GetStopIndex(stop_from), GetStopIndex(stop_to));
}

const graph::Edge<double>& Router::GetEdge(size_t edge_id) const{
    return tc_graph_.GetEdge(edge_id);
}

const EdgeInfo& Router::GetEdgeInfo(size_t edge_id) const{
    return graph_edge_to_info_.at(edge_id);
}


const graph::DirectedWeightedGraph<double>& Router::ReturnInitializedGraph(){
    CreateGraph();
    AddStopsEdgeToGraph();
    AddStopToStopEdgeToGraph();
    return tc_graph_;
}

void Router::CreateGraph(){
    size_t i = 0;
    for (const auto& stop_name : tc_.GetAllStopNames()){
        stopptr_to_graph_[tc_.GetStopInfo(stop_name)] = i;
        i += 2;
    }
}
void Router::AddStopsEdgeToGraph(){
    for (auto stop_name : tc_.GetAllStopNames()){
        size_t index = GetStopIndex(stop_name);
        auto edge_id = tc_graph_.AddEdge({index, index + 1, settings_.bus_wait_time*1.0});
        AddEdgeInfo(edge_id, {EdgeType::WAIT, stop_name});
    }
}

void Router::AddStopToStopEdgeToGraph(){
    for (auto bus_name : tc_.GetAllBusNames()){
        auto route =  tc_.GetBusRoute(bus_name);
        if (tc_.BusIsRoundtrip(bus_name)){
            AddBusRouteEdgesToGraph(route.begin(), route.end(), bus_name);
        } else {
            auto it_middle = route.begin() + route.size() / 2;
            AddBusRouteEdgesToGraph(route.begin(), it_middle + 1, bus_name);
            AddBusRouteEdgesToGraph(it_middle, route.end(), bus_name);
        }
    }
}

void Router::AddEdgeInfo(size_t edge_id, EdgeInfo edge_info){
    graph_edge_to_info_[edge_id] = std::move(edge_info);
}
void Router::AddStopToStopEdge(Stop* from, Stop* to, size_t edge_id){
    pairstops_to_edge_id_[std::make_pair(from, to)] = edge_id;
}

size_t Router::GetStopIndex(std::string_view stop_name) const{
    return stopptr_to_graph_.at(tc_.GetStopInfo(stop_name));
}
std::optional<size_t> Router::GetPairStopsEdgeId(Stop* from, Stop* to){
     return pairstops_to_edge_id_.count(std::make_pair(from, to)) == 0 ? std::nullopt : std::optional<size_t>(pairstops_to_edge_id_.at(std::make_pair(from, to)));
}

size_t Router::StopToStopHasher::operator() (const std::pair<Stop*, Stop*>& pairstops) const{
    std::hash<Stop*> hasher_;
    return static_cast<size_t>(hasher_(pairstops.first) + 1801 * hasher_(pairstops.second));
}

} // namespace router
} // namespace tc
