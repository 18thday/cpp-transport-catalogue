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

//struct SerializationSettings{
//    std::string filename;
//};

// Serialization
void Serialize(const tc::TransportCatalogue& tc, const tc::renderer::RenderSettings& render_set,
               const tc::router::RoutingSettings& routing_set, const std::string& filename);

tc_serialization::TransportCatalogue SerializeTransportCatalogue(const tc::TransportCatalogue& tc);
void SerializeStop(tc_serialization::TransportCatalogue& tc_pb, const tc::TransportCatalogue& tc);
void SerializeBus(tc_serialization::TransportCatalogue& tc_pb, const tc::TransportCatalogue& tc);

tc_serialization::Color SerializeSVGColor(const svg::Color& from_color);
tc_serialization::RenderSettings SerializeRenderSettings(const tc::renderer::RenderSettings& render_set);

tc_serialization::RoutingSettings SerializeRoutingSettings(const tc::router::RoutingSettings& routing_set);

// Deserialization
void Deserialize(tc::TransportCatalogue& tc, tc::renderer::RenderSettings& render_set,
                 tc::router::RoutingSettings& routing_set, const std::string& filename);

tc::TransportCatalogue DeserializeTransportCatalogue(const tc_serialization::TransportCatalogue& tc_pb);
void DeserializeStop(tc::TransportCatalogue& tc, const tc_serialization::TransportCatalogue& tc_pb);
void DeserializeBus(tc::TransportCatalogue& tc, const tc_serialization::TransportCatalogue& tc_pb);

tc::renderer::RenderSettings DeserializeRenderSettings(const tc_serialization::RenderSettings& render_set_pb);
svg::Color DeserializeSVGColor(tc_serialization::Color color);

tc::router::RoutingSettings DeserializeRoutingSettings(const tc_serialization::RoutingSettings& routing_set_pb);

} // namespace serialization
} // namespace tc
