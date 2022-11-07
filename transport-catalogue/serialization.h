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

struct SerializationSettings{
    std::string filename;
};

// Serialization
void Serialize(const tc::TransportCatalogue& tc, const tc::renderer::RenderSettings& rs,
               const tc::router::RoutingSettings& routing_s, const std::string& filename);

void SerializeStop(tc_serialization::TC& tc_pb, const tc::TransportCatalogue& tc);
void SerializeBus(tc_serialization::TC& tc_pb, const tc::TransportCatalogue& tc);

tc_serialization::Color SerializeSVGColor(const svg::Color& from_color);
void SerializeRenderSettings(tc_serialization::TC& tc_pb, const tc::renderer::RenderSettings& rs);

void SerializeRoutingSettings(tc_serialization::TC& tc_pb, const tc::router::RoutingSettings& routing_s);

// Deserialization
tc::TransportCatalogue DeserializeTransportCatalogue(const tc_serialization::TC& tc_pb);
void DeserializeStop(tc::TransportCatalogue& tc, const tc_serialization::TC& tc_pb);
void DeserializeBus(tc::TransportCatalogue& tc, const tc_serialization::TC& tc_pb);

tc::renderer::RenderSettings DeserializeRenderSettings(const tc_serialization::TC& tc_pb);
svg::Color DeserializeSVGColor(tc_serialization::Color color);

tc::router::RoutingSettings DeserializeRoutingSettings(const tc_serialization::TC& tc_pb);

} // namespace serialization
} // namespace tc
