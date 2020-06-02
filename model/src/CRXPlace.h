/*
 * CRXPlace.h
 *
 *  Created on: Jul 22, 2016
 *      Author: nick
 */

#ifndef CRXPLACE_H_
#define CRXPLACE_H_

#include <string>
#include <vector>
#include <memory>
#include <map>

#include "chi_sim/PlaceId.h"

#include "GeoPoint.h"
#include "utils.h"
#include "PropensityMap.h"

namespace crx {

class CRXPerson;

class PlaceParameters {

private:

    const int id_, zipcode_, rank_;
    const std::string type_;
    const GeoPoint location_;

    PlaceParameters(int id, int zipcode, int rank, const std::string& type,
            const GeoPoint& location);

public:

    class Builder;

    int id() const {
        return id_;
    }

    int zipcode() const {
        return zipcode_;
    }

    int rank() const {
        return rank_;
    }

    std::string type() const {
        return type_;
    }

    GeoPoint location() const {
        return location_;
    }
};

class PlaceParameters::Builder {

private:
    int id_, zipcode_, rank_;
    std::string type_;
    GeoPoint location_;

public:
    Builder();

    Builder& id(int id);
    Builder& zipcode(int zip);
    Builder& rank(int rank);
    Builder& type(const std::string& type);
    Builder& location(const GeoPoint& location);

    PlaceParameters build();

};

class CRXPlace {

protected:
    std::string type_;
    int zip;
    GeoPoint loc;
    chi_sim::PlaceId id_;
    bool home_work_school;

public:
    using const_iterator = std::vector<int>::const_iterator;

    CRXPlace(const PlaceParameters& params);
    virtual ~CRXPlace();

    /**
     * Reset for next time step.
     */
    virtual void reset() {}

    const chi_sim::PlaceId placeId() const {
        return id_;
    }

    const GeoPoint location() const {
        return loc;
    }

    int zipcode() const {
        return zip;
    }

    const std::string type() {
        return type_;
    }

    bool isHomeWorkSchool() const;

    virtual void addPerson(const std::shared_ptr<CRXPerson>& person, int act_type);
    virtual void run(PropensityMap& propensity_map);

    virtual const_iterator serviceCodesBegin();
    virtual const_iterator serviceCodesEnd();

    virtual size_t size() const {
        return 0;
    }
};

extern const std::shared_ptr<CRXPlace> NULL_PLACE;


} /* namespace crx */

#endif /* CRXPLACE_H_ */
