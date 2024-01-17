#pragma once

#include <cmath>

namespace TransportSystem {

struct Coordinates {
    double lat;
    double lng;
    bool operator==(const Coordinates& other) const {
        return lat == other.lat && lng == other.lng;
    }
    bool operator!=(const Coordinates& other) const {
        return !(*this == other);
    }
};

constexpr double EarthRadius = 6371000.0;
constexpr double Pi = 3.1415926535;

inline double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    if (from == to) {
        return 0;
    }
    static const double dr = Pi / 180.;
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
           * EarthRadius;
}

} // namespace TransportSystem
