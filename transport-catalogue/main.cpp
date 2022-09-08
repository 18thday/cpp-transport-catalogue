#include "transport_catalogue.h"
#include "json.h"
#include "json_reader.h"

#include "input_reader.h"
#include "stat_reader.h"

#include <string>
#include <iostream>
#include <utility>

using namespace std;

int main() {
    tc::TransportCatalogue tc;

    tc::reader::ReadJSON(tc, std::cin);

    return 0;
}
