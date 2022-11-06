#include "request_handler.h"

#include <iostream>

#include "transport_catalogue.h"
#include "json.h"
#include "json_reader.h"
#include "serialization.h"

void MakeBase(){
    tc::TransportCatalogue tc;
    tc::reader::MakeBaseFromJSON(tc, std::cin);
}

void ProcessRequests(){
	tc::reader::ProcessRequestFromJSON(std::cin);

}
