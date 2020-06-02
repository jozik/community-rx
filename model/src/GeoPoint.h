/*
 * Point.h
 *
 *  Created on: Oct 24, 2017
 *      Author: nick
 */

#ifndef GEOPOINT_H_
#define GEOPOINT_H_

namespace crx {

class GeoPoint {

private:
    double lat_, lon_;

public:
    GeoPoint(double lat, double lon);
    ~GeoPoint();

    double latitude() const {
        return lat_;
    }

    double longitude() const {
        return lon_;
    }

    double distance(const GeoPoint& other) const;
};

} /* namespace crx */

#endif /* GEOPOINT_H_ */
