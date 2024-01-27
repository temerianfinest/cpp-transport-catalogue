#pragma once

#include <cmath>

namespace Geo
{
    const int EARTH_RADIUS_M = 6371000;

    struct Coordinates 
    {
        double lat = 0;
        double lng = 0;

        bool operator==(const Coordinates& other) const 
        {
            return lat == other.lat && lng == other.lng;
        }
        bool operator!=(const Coordinates& other) const 
        {
            return !(*this == other);
        }
    };

    double ComputeDistance(Coordinates from, Coordinates to);
}