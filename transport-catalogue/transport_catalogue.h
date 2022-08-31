#pragma once

#include <deque>
#include <functional>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "geo.h"
#include "domain.h"

namespace tc{


struct StopRequest{
    bool have_stop = false;
    std::set<std::string_view> all_buses;
};

struct RouteStatistics{
    size_t stops = 0;
    size_t unique_stops = 0;
    int route_length = 0;
    double route_length_geo = 0;
    double curvature = 0;
};

class TransportCatalogue{
public:
    void AddStop(const std::string& name, double latitude, double longitude);
    void AddBus(const std::string& bus_name, const std::vector<std::string_view>& stops_for_bus, bool is_roundtrip);

    std::set<std::string_view> GetAllBusNames() const;
    bool BusIsRoundtrip(std::string_view bus_name) const;
    const std::vector<Stop*>& GetBusRoute(std::string_view bus_name) const;

    std::set<std::string_view> GetAllStopNames() const;
    const Stop* GetStopInfo(std::string_view stop_name) const;
    StopRequest GetBusForStop(std::string_view stop_name) const;
    bool StopHaveBus(std::string_view stop_name) const;

    void SetDistance(std::string_view stopname_from, std::string_view stopname_to, int distance);
    int GetDistance(std::string_view stopname_from, std::string_view stopname_to) const;
    int GetDistance(Stop* stopptr_from, Stop* stopptr_to) const;

    RouteStatistics GetStatistics(std::string_view bus_name)const;

private:
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;

    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;

    std::unordered_map<Stop*, std::set<std::string_view>> stopptr_to_buses_;

    struct StopToStopHasher{
        size_t operator() (const std::pair<Stop*, Stop*>& pairstops) const;
    };
    std::unordered_map<std::pair<Stop*, Stop*>, int, StopToStopHasher> pairstops_to_dist_;

    std::vector<Stop*> dummy_stop;
};

} // namespace tc
