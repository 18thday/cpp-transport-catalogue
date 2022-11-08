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
    tc::renderer::RenderSettings render_set;
    tc::router::RoutingSettings routing_set;

    tc::reader::MakeBaseFromJSON(tc, render_set, routing_set, main_node);

    std::string filename = tc::reader::ReadSerializationSettingsFromJSON(main_node);
    tc::serialization::Serialize(tc, render_set, routing_set, filename);
}

void ProcessRequests(){
    json::Node main_node = tc::reader::LoadJSON(std::cin);

    std::string filename = tc::reader::ReadSerializationSettingsFromJSON(main_node);

    tc::TransportCatalogue tc;
    tc::renderer::RenderSettings render_set;
    tc::router::RoutingSettings routing_set;

    tc::serialization::Deserialize(tc, render_set, routing_set, filename);

    tc::reader::ProcessRequestFromJSON(tc, render_set, routing_set, main_node);
}
