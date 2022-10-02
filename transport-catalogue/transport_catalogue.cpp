#include "transport_catalogue.h"

using namespace std;

#include <cassert>
#include <iostream>

namespace tc{

void TransportCatalogue::AddStop(const std::string& name, double latitude, double longitude){
    stops_.push_back({name, {latitude, longitude} });
    stopname_to_stop_.insert({stops_.back().name, &(stops_.back())});
}

void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string_view>& stops_for_bus, bool is_roundtrip){
    Bus bus;
    bus.name = name;
    bus.stops.reserve(stops_for_bus.size());
    bus.is_roundtrip = is_roundtrip;

    for(auto& stop : stops_for_bus){
        bus.stops.push_back(stopname_to_stop_.at(stop));
    }

    buses_.emplace_back(move(bus));// move
    busname_to_bus_.insert({buses_.back().name, &(buses_.back())});

    for(auto& stop : stops_for_bus){
        stopptr_to_buses_[stopname_to_stop_.at(stop)].insert(buses_.back().name);
    }
}

std::set<std::string_view> TransportCatalogue::GetAllBusNames() const{
    std::set<std::string_view> result;
    for (const auto& [bus_name, _] : busname_to_bus_){
        result.insert(bus_name);
    }
    return result;
}

bool TransportCatalogue::BusIsRoundtrip(std::string_view bus_name) const{
    return busname_to_bus_.at(bus_name)->is_roundtrip;

}

const vector<Stop*>& TransportCatalogue::GetBusRoute(string_view bus_name) const{
    if (busname_to_bus_.count(bus_name) != 0){
        return busname_to_bus_.at(bus_name)->stops;
    }
    return dummy_stop;
}


std::set<std::string_view> TransportCatalogue::GetAllStopNames() const{
    std::set<std::string_view> result;
    for (const auto& [stop_name, _] : stopname_to_stop_){
        result.insert(stop_name);
    }
    return result;
}
const Stop* TransportCatalogue::GetStopInfo(std::string_view stop_name) const{
    if (stopname_to_stop_.count(stop_name) == 0){
        return nullptr;
    }
    return stopname_to_stop_.at(stop_name);
}

StopRequest TransportCatalogue::GetBusForStop (string_view stop_name) const{
    StopRequest result;
    if (stopname_to_stop_.count(stop_name) != 0){
        result.have_stop = true;
        if (stopptr_to_buses_.count(stopname_to_stop_.at(stop_name)) != 0){
            result.all_buses = stopptr_to_buses_.at(stopname_to_stop_.at(stop_name));
        }
    }
    return result;
}

bool TransportCatalogue::StopHaveBus(std::string_view stop_name) const{
    return (stopptr_to_buses_.count(stopname_to_stop_.at(stop_name)) != 0);
}

void TransportCatalogue::SetDistance(std::string_view stopname_from, std::string_view stopname_to, int distance){
//    cerr << stopname_from << " "s << stopname_to << " "s << distance << endl;
    assert(!((stopname_to_stop_.count(stopname_from) == 0) || (stopname_to_stop_.count(stopname_to) == 0)));
    pairstops_to_dist_[{stopname_to_stop_.at(stopname_from), stopname_to_stop_.at(stopname_to)}] = distance;
}

int TransportCatalogue::GetDistance(std::string_view stopname_from, std::string_view stopname_to) const{
    assert(!((stopname_to_stop_.count(stopname_from) == 0) || (stopname_to_stop_.count(stopname_to) == 0)));
    return GetDistance(stopname_to_stop_.at(stopname_from), stopname_to_stop_.at(stopname_to));
}
int TransportCatalogue::GetDistance(Stop* stopptr_from, Stop* stopptr_to) const{
    if (pairstops_to_dist_.count({stopptr_from, stopptr_to}) == 0){
        return pairstops_to_dist_.at({stopptr_to, stopptr_from});
    }
    return pairstops_to_dist_.at({stopptr_from, stopptr_to});
}

RouteStatistics TransportCatalogue::GetStatistics(std::string_view bus_name) const{
    RouteStatistics result;
    const auto& route = GetBusRoute(bus_name);
    if (route.size() == 0){
        return result;
    }

    result.stops = route.size();

    set<Stop*> unique_stops(route.begin(), route.end());
    result.unique_stops = unique_stops.size();

    for (auto it = route.begin(); it < route.end() - 1; ++it){
        result.route_length_geo += std::abs(ComputeDistance((*it)->coordinates, (*(it + 1))->coordinates));
        result.route_length += GetDistance((*it)->name, (*(it+1))->name);
    }

    result.curvature = result.route_length_geo != 0 ? result.route_length / result.route_length_geo : 0;

    return result;
}

size_t TransportCatalogue::GetStopCount() const{
    return stopname_to_stop_.size();
}

void TransportCatalogue::CreateGraph(){
    size_t i = 0;
    for (const auto [_, stopptr] : stopname_to_stop_){
        stopptr_to_graph_[stopptr] = i;
        i += 2;
    }
}

void TransportCatalogue::AddEdgeInfo(size_t edge_id, EdgeInfo edge_info){
    graph_edge_to_info_[edge_id] = std::move(edge_info);
}

void TransportCatalogue::AddStopToStopEdge(Stop* from, Stop* to, size_t edge_id){
    pairstops_to_edge_id_[make_pair(from, to)] = edge_id;
}

size_t TransportCatalogue::GetStopIndex(std::string_view stop_name) const{
    return stopptr_to_graph_.at(GetStopInfo(stop_name));
}

optional<size_t> TransportCatalogue::GetPairStopsEdgeId(Stop* from, Stop* to) {
    return pairstops_to_edge_id_.count(make_pair(from, to)) == 0 ? std::nullopt : optional<size_t>(pairstops_to_edge_id_.at(make_pair(from, to)));
}

EdgeInfo TransportCatalogue::GetEdgeInfo(size_t edge_id) const {
    return graph_edge_to_info_.at(edge_id);
}

size_t TransportCatalogue::StopToStopHasher::operator ()(const std::pair<Stop*, Stop*>& pairstops) const{
    hash<Stop*> hasher_;
    return static_cast<size_t>(hasher_(pairstops.first) + 1801 * hasher_(pairstops.second));
}

} // namespace tc
