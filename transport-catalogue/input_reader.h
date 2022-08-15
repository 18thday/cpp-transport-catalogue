#pragma once

#include "transport_catalogue.h"

#include <string>
#include <string_view>
#include <vector>
#include <utility>

namespace tc{
namespace query{

void ReadInput(TransportCatalogue& tc);
void ParseInput(TransportCatalogue& tc, const std::vector<std::string>& queries);

namespace service{

std::vector<std::string_view> Split(std::string_view query);
std::pair<int, std::string_view> ParseDistanceToStop(std::string_view query);
std::string_view CutStrViewBeforeSeparator(std::string_view& query, char separator);

} // namespace service

namespace data{

void AddStop(TransportCatalogue& tc, const std::vector<std::string_view>& query);
void AddStopsDistance(TransportCatalogue& tc, const std::vector<std::string_view>& query);
void AddBus(TransportCatalogue& tc, const std::vector<std::string_view>& query);

} // namespace data

} // namespace query
} // namespace tc
