#pragma once

#include "json.h"
#include "json_builder.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "graph.h"
#include "router.h"
#include "transport_router.h"
#include "router.h"
#include "serialization.h"

#include <string>

namespace tc {
namespace reader {

void ReadJSON(tc::TransportCatalogue& tc, std::istream& input = std::cin);
json::Node LoadJSON(std::istream& input);

renderer::RenderSettings ReadRenderSettingsFromJSON(const json::Dict& db);
svg::Color ReadSVGColorFromJSON(const json::Node& from_color);
std::string ReadSerializationSettingsFromJSON(const json::Node& main_node);

bool MakeBaseFromJSON(tc::TransportCatalogue& tc, tc::renderer::RenderSettings& render_s,
                      tc::router::RoutingSettings& routing_s, const json::Node& main_node);

bool ProcessRequestFromJSON(const tc::TransportCatalogue& tc, const tc::renderer::RenderSettings& render_s,
                            const tc::router::RoutingSettings& routing_s, const json::Node& main_node);

namespace handler {

void PerformBaseRequests(tc::TransportCatalogue& tc, const json::Dict& db);
void PerformStatRequests(const tc::TransportCatalogue& tc, const json::Dict& db,
                         const renderer::MapRenderer& mr, const tc::router::Router& router);

//  BaseRequest Handlers
void AddStop(tc::TransportCatalogue& tc, const json::Dict& request);
void AddStopDistances(tc::TransportCatalogue& tc, const json::Dict& request);
void AddBus(tc::TransportCatalogue& tc, const json::Dict& request);

// Routing handler
tc::router::RoutingSettings PerformRoutingSettings(const json::Dict& db);

//  StatRequest Handlers
void GetStatAnswer(const tc::TransportCatalogue& tc, const json::Dict& request, const renderer::MapRenderer& render_settings, json::Builder& bjson,
                   const tc::router::Router& router);

void RouteStatisticsToDictConvertion(json::Builder& bjson, const tc::RouteStatistics& stat);
void StopRequestToDictConvertion(json::Builder& bjson, const tc::StopRequest& stop);
std::ostringstream& MapRequest(std::ostringstream& str_stream, const tc::TransportCatalogue& tc, const renderer::MapRenderer& render_settings);

//  Render
svg::Document RenderMap(const tc::TransportCatalogue& tc, const renderer::MapRenderer& render_settings);

} // namespace handler

} // namespace reader
} // namespace tc
