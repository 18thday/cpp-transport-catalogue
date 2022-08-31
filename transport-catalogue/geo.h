#pragma once

#include <cmath>

namespace tc{
namespace geo{

static const double THRESHOLD = 1e-6;

struct Coordinates {
    double lat;
    double lng;
    bool operator==(const Coordinates& other) const {
        return std::abs(lat - other.lat) < THRESHOLD && std::abs(lng - other.lng) < THRESHOLD;
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
};

double ComputeDistance(Coordinates from, Coordinates to);

} // namespace geo
} // namespace tc
