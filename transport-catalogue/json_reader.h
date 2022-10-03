#pragma once

#include "json.h"
#include "json_builder.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "graph.h"
#include "router.h"
#include "transport_router.h"

namespace tc {
namespace reader {

void ReadJSON(tc::TransportCatalogue& tc, std::istream& input = std::cin);
renderer::RenderSettings ReadRenderSettingsFromJSON(const json::Dict& db);
svg::Color ReadSVGColorFromJSON(const json::Node& from_color);

namespace handler {

void PerformBaseRequests(tc::TransportCatalogue& tc, const json::Dict& db);
void PerformStatRequests(tc::TransportCatalogue& tc, const json::Dict& db,
                         const renderer::MapRenderer& mr, const tc::router::Router& router);

//  BaseRequest Handlers
void AddStop(tc::TransportCatalogue& tc, const json::Dict& request);
void AddStopDistances(tc::TransportCatalogue& tc, const json::Dict& request);
void AddBus(tc::TransportCatalogue& tc, const json::Dict& request);

// Routing handler
tc::router::RoutingSettings PerformRoutingSettings(const json::Dict& db);

//  StatRequest Handlers
void GetStatAnswer(tc::TransportCatalogue& tc, const json::Dict& request, const renderer::MapRenderer& render_settings, json::Builder& bjson,
                   const tc::router::Router& router);

void RouteStatisticsToDictConvertion(json::Builder& bjson, const tc::RouteStatistics& stat);
void StopRequestToDictConvertion(json::Builder& bjson, const tc::StopRequest& stop);
std::ostringstream& MapRequest(std::ostringstream& str_stream, tc::TransportCatalogue& tc, const renderer::MapRenderer& render_settings);

//  Render
svg::Document RenderMap(const tc::TransportCatalogue& tc, const renderer::MapRenderer& render_settings);

} // namespace handler

} // namespace reader
} // namespace tc
