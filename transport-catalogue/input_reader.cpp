#include "input_reader.h"

#include <algorithm>
#include <charconv>
#include <iostream>
#include <iomanip>
#include <string>
#include <string_view>

using namespace std;

namespace tc{
namespace query{

//enum QueryType{
//    BUS,
//    STOP
//};
//
//struct QueryInput{
//    QueryType type;
//    std::string query;
//};

void ReadInput(TransportCatalogue& tc){
    int in_query;
    cin >> in_query;
    cin.ignore();

    vector<string> queries;
    queries.reserve(in_query);
    while(in_query != 0){
        string line;
        getline(cin, line);

        queries.emplace_back(move(line));
        --in_query;
    }
    ParseInput(tc, queries);
}

void ParseInput(TransportCatalogue& tc, const vector<string>& queries){
    vector<vector<string_view>> q;
    for(auto& query : queries){
        q.push_back(service::Split(query));
    }

    // Add all stops
    for(auto& item : q){
        if (item.at(0) == "Stop"sv){
            data::AddStop(tc, item);
        }
    }
    // Add distances between stops
    for(auto& item : q){
        if (item.at(0) == "Stop"sv){
            data::AddStopsDistance(tc, item);
        }
    }
    // Add buses
    for(auto& item : q){
        if (item[0] == "Bus"sv){
            data::AddBus(tc, item);
        }
    }
}

namespace service{

vector<string_view> Split(string_view query){
    vector<string_view> result;

    result.push_back(CutStrViewBeforeSeparator(query, ' '));
    result.push_back(CutStrViewBeforeSeparator(query, ':'));

    char separator = '\n';
    if (result[0] == "Stop"sv){
        separator = ',';
    } else if (result[0] == "Bus"sv){
        separator = (query.find('>') != query.npos) ? '>': '-' ;
        size_t pos = query.find(separator);
        result.push_back(query.substr(pos, 1)); // add separator
    }

    while (true){
        result.push_back(CutStrViewBeforeSeparator(query, separator));
        if (query.size() == 0){
            break;
        }
    }
    return result;
}

pair<int, string_view> ParseDistanceToStop(string_view query){
    int distance = stoi(string(CutStrViewBeforeSeparator(query, 'm')));

    query.remove_prefix(query.find_first_not_of(' '));
    query.remove_prefix(2); // remove "to"
    query.remove_prefix(query.find_first_not_of(' '));

    return {distance, query.substr(0, min(query.find_last_not_of(' ') + 1, query.size()))};
}

string_view CutStrViewBeforeSeparator(string_view& query, char separator){
    query.remove_prefix(query.find_first_not_of(' '));
    size_t pos = min(query.find(separator), query.size());
    string_view q = query.substr(0, pos);
    query.remove_prefix(min(pos + 1, query.size()));
    q = q.substr(0, min(q.find_last_not_of(' ') + 1, q.size()));
    return q;
}

} // namespace service

namespace data{

void AddStop(TransportCatalogue& tc, const vector<string_view>& query){
//    string_view sv_lat = query[2];
//    string_view sv_lng = query[3];
//    double lat, lng;
//    from_chars(sv_lat.begin(), sv_lat.begin() + sv_lat.size(), lat);
//    from_chars(sv_lng.begin(), sv_lng.begin() + sv_lng.size(), lng);

    // query[1] - stop_name ; query[2] - lat ; query[3] - lng
    tc.AddStop(string(query[1]), std::stod(string(query[2])), std::stod(string(query[3])));
}

void AddStopsDistance(TransportCatalogue& tc, const vector<string_view>& query){
    if (query.size() <= 4){
        return;
    }

    for(size_t i = 4; i < query.size(); ++i){
        auto dist_stopname_to = service::ParseDistanceToStop(query[i]);
        tc.SetDistance(query[1], dist_stopname_to.second, dist_stopname_to.first);
    }
}

void AddBus(TransportCatalogue& tc, const vector<string_view>& query){
    string_view bus_name = query[1];
    if (query[2] == ">"sv){
        vector<string_view> stops_for_bus(query.begin() + 3, query.end());
        tc.AddBus(string(bus_name), stops_for_bus, true);
        return;
    }
    if (query[2] == "-"sv){
        vector<string_view> stops_for_bus;
        stops_for_bus.reserve((query.size() - 3) * 2);
        for(auto it = query.begin() + 3; it < query.end(); ++it){
            stops_for_bus.push_back(*it);
        }
        for(auto it = query.rbegin() + 1; it < query.rend() - 3; ++it){
            stops_for_bus.push_back(*it);
        }
        tc.AddBus(string(bus_name), stops_for_bus, false);
        return;
    }
}

} // namespace data

} // namespace query
} // namespace tc
