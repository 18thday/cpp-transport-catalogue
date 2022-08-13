#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

#include <string>
#include <iostream>
#include <utility>

using namespace std;

int main() {
    tc::TransportCatalogue tc;

    tc::query::ReadInput(tc);
    tc::query::ReadStat(tc);
    return 0;
}
