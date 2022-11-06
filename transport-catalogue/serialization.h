#pragma once
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <string>

#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>
#include <svg.pb.h>
#include <transport_router.pb.h>

namespace tc {
namespace serialization {

void Serialize(const tc::TransportCatalogue& tc, const std::string& filename);

void Serialize(const json::Dict& db, const std::string& filename);
void SerializeStop(tc_serialization::TC& TC, const json::Dict& request);
void SerializeBus(tc_serialization::TC& TC, const json::Dict& request);

tc_serialization::Color SerializeSVGColor(const json::Node& from_color);
void SerializeRenderSettings(tc_serialization::TC& TC, const json::Dict& db);

void SerializeRoutingSettings(tc_serialization::TC& TC, const json::Dict& db);

//tc::TransportCatalogue Deserialize(const std::string& filename);
tc::TransportCatalogue DeserializeTransportCatalogue(const tc_serialization::TC& TC);
void DeserializeStop(tc::TransportCatalogue& tc, const tc_serialization::TC& TC);
void DeserializeBus(tc::TransportCatalogue& tc, const tc_serialization::TC& TC);

tc::renderer::RenderSettings DeserializeRenderSettings(const tc_serialization::TC& TC);
svg::Color DeserializeSVGColor(tc_serialization::Color color);

tc::router::RoutingSettings DeserializeRoutingSettings(const tc_serialization::TC& TC);

} // namespace serialization
} // namespace tc
