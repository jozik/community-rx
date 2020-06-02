/*
 * Point.cpp
 *
 *  Created on: Oct 24, 2017
 *      Author: nick
 */

#include <cmath>

#include "GeoPoint.h"

namespace crx {

const double EARTH_RADIUS_MILES = 3958.76;
const double RADIANS_PER_DEGREE = M_PI / 180.0;

GeoPoint::GeoPoint(double latitude, double longitude) : lat_{latitude}, lon_{longitude} {
}

GeoPoint::~GeoPoint() {
}

double GeoPoint::distance(const GeoPoint& other) const {
    double other_lat_rad = other.lat_ * RADIANS_PER_DEGREE;
    double other_lon_rad = other.lon_ * RADIANS_PER_DEGREE;
    double lat_rad = lat_ * RADIANS_PER_DEGREE;
    double lon_rad = lon_ * RADIANS_PER_DEGREE;

    /* Equirectangular approximation (good for close distances) */
    double xunit = (other_lon_rad - lon_rad) * cos((lat_rad + other_lat_rad) / 2);
    double yunit = other_lat_rad - lat_rad;
    return sqrt(xunit * xunit + yunit * yunit) * EARTH_RADIUS_MILES;
}

} /* namespace crx */
