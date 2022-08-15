#pragma once

#include "transport_catalogue.h"

#include <string_view>

namespace tc{
namespace query{

void ReadStat(const TransportCatalogue& tc);
void ParseStat(const TransportCatalogue& tc, const std::vector<std::string>& queries);

} // namespace query

namespace print{

void PrintBus(const TransportCatalogue& tc, std::string_view bus_name);
void PrintStop(const TransportCatalogue& tc, std::string_view stop_name);

} // namespace print
} // namespace tc
