#include "request_handler.h"

#include <iostream>
#include <string>
#include <fstream>

#include "transport_catalogue.h"
#include "json.h"
#include "json_reader.h"
#include "serialization.h"

void MakeBase(){
    json::Node main_node = tc::reader::LoadJSON(std::cin);

    tc::TransportCatalogue tc;
    tc::renderer::RenderSettings render_s;
    tc::router::RoutingSettings routing_s;
    tc::reader::MakeBaseFromJSON(tc, render_s, routing_s, main_node);

    std::string filename = tc::reader::ReadSerializationSettingsFromJSON(main_node);
    tc::serialization::Serialize(tc, render_s, routing_s, filename);
}

void ProcessRequests(){
    json::Node main_node = tc::reader::LoadJSON(std::cin);
    std::string filename = tc::reader::ReadSerializationSettingsFromJSON(main_node);

    tc_serialization::TC tc_pb;

    std::ifstream ifs(filename, std::ios_base::binary);
    if (!tc_pb.ParseFromIstream(&ifs)) {
        return;
    }

    tc::TransportCatalogue tc = tc::serialization::DeserializeTransportCatalogue(tc_pb);
    tc::renderer::RenderSettings render_s = tc::serialization::DeserializeRenderSettings(tc_pb);
    tc::router::RoutingSettings routing_s = tc::serialization::DeserializeRoutingSettings(tc_pb);

    tc::reader::ProcessRequestFromJSON(tc, render_s, routing_s, main_node);
}
