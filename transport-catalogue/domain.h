#pragma once

#include <string>
#include <vector>

#include "geo.h"

namespace tc {

struct Stop{
    bool operator==(const Stop& other) const {
        return name == other.name;
    }

    std::string name;
    tc::geo::Coordinates coordinates;
};

struct Bus{
    bool operator==(const Bus& other) const {
        return name == other.name;
    }

    std::string name;
    std::vector<Stop*> stops;
    bool is_roundtrip;
};

} // namespace tc
