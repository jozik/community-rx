/*
 * CRXPlace.cpp
 *
 *  Created on: Jul 22, 2016
 *      Author: nick
 */

#include "CRXPlace.h"
#include "CRXPerson.h"

#include "chi_sim/CSVReader.h"

#include "constants.h"

namespace crx {

PlaceParameters::PlaceParameters(int id, int zipcode, int rank, const std::string& type,
        const GeoPoint& location) : id_(id), zipcode_(zipcode), rank_(rank),
        type_(type), location_(location) {
}


PlaceParameters::Builder::Builder() : id_(-1), zipcode_(-1), rank_(-1),
        type_(""), location_(0, 0) {
}

PlaceParameters::Builder& PlaceParameters::Builder::id(int id) {
    id_ = id;
    return *this;
}

PlaceParameters::Builder& PlaceParameters::Builder::zipcode(int zip) {
    zipcode_ = zip;
    return *this;
}
PlaceParameters::Builder& PlaceParameters::Builder::rank(int rank) {
    rank_ = rank;
    return *this;
}

PlaceParameters::Builder& PlaceParameters::Builder::type(const std::string& type) {
    type_ = type;
    return *this;
}

PlaceParameters::Builder& PlaceParameters::Builder::location(const GeoPoint& location) {
    location_ = location;
    return *this;
}

PlaceParameters PlaceParameters::Builder::build() {
    PlaceParameters p(id_,  zipcode_, rank_, type_, location_);
    return p;
}

CRXPlace::CRXPlace(const PlaceParameters& params) :
        type_{params.type()},  zip{params.zipcode()}, loc{params.location()}, id_{params.id(), params.rank()},
        home_work_school{type_ == WORKPLACE_TYPE || type_ == HOUSEHOLD_TYPE || type_ == SCHOOL_TYPE}
       {
}

CRXPlace::~CRXPlace() {
}


void CRXPlace::addPerson(const std::shared_ptr<CRXPerson>& person, int act_type) {
    throw std::domain_error("Person added to non-local place!");
}

void CRXPlace::run(PropensityMap& propensity_map) {
    throw std::domain_error("run() called non-local place");
}

CRXPlace::const_iterator CRXPlace::serviceCodesBegin() {
    throw std::domain_error("Service Codes requested from non-local place");
}

CRXPlace::const_iterator CRXPlace::serviceCodesEnd() {
    throw std::domain_error("Service Codes requested from non-local place");
}

bool CRXPlace::isHomeWorkSchool() const {
    return home_work_school;
}


PlaceParameters::Builder builder;
const std::shared_ptr<CRXPlace> NULL_PLACE = std::make_shared<CRXPlace>(builder.type("null").
        id(chi_sim::NULL_PLACE_ID.place_id).rank(chi_sim::NULL_PLACE_ID.rank).zipcode(0).
        location(GeoPoint(0, 0)).build());

} /* namespace crx */
