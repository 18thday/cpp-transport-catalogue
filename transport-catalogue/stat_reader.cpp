#include "stat_reader.h"
#include "input_reader.h"

#include <cmath>
#include <iostream>
#include <iomanip>
#include <set>
#include <string>
#include <string_view>

using namespace std;

namespace tc{
namespace query{

void ReadStat(const TransportCatalogue& tc){
    int stat_query;
    cin >> stat_query;
    cin.ignore();

    vector<string> queries;
    queries.reserve(stat_query);
    while(stat_query != 0){
        string line;
        getline(cin, line);

        queries.emplace_back(move(line));
        --stat_query;
    }
    ParseStat(tc, queries);
}

void ParseStat(const TransportCatalogue& tc, const vector<string>& queries){
    for(string_view query : queries){
        string_view q = service::CutStrViewBeforeSeparator(query, ' ');
        if (q == "Bus"s){
            print::PrintBus(tc, service::CutStrViewBeforeSeparator(query, '\n'));
        }
        if (q == "Stop"s){
            print::PrintStop(tc, service::CutStrViewBeforeSeparator(query, '\n'));
        }
    }
}

} // namespace query

namespace print{

void PrintBus(const TransportCatalogue& tc, std::string_view bus_name){
    RouteStatistics stat = tc.GetStatistics(bus_name);
    if (stat.stops == 0){
        cout << "Bus "s << bus_name << ": not found"s << endl;
        return;
    }
    cout << "Bus "s << bus_name << ": "s
                    << stat.stops << " stops on route, "s
                    << stat.unique_stops << " unique stops, "s
                    << setprecision(6) << stat.route_length << " route length, "s
                    << setprecision(6) << stat.curvature << " curvature"s << endl;
}

void PrintStop(const TransportCatalogue& tc, std::string_view stop_name){
    const auto& result = tc.GetBusForStop(stop_name);
    if (!result.have_stop){
        cout << "Stop "s << stop_name << ": not found"s << endl;
        return;
    }
    if (result.all_buses.empty()){
        cout << "Stop "s << stop_name << ": no buses"s << endl;
        return;
    }

    cout << "Stop "s << stop_name << ": buses"s;
    for (auto bus_name : result.all_buses){
        cout << " "s << bus_name;
    }
    cout << endl;
}

} // namespace print
} // namespace tc
