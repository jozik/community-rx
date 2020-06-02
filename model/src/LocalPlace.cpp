/*
 * LocalPlace.cpp
 *
 *  Created on: Oct 24, 2017
 *      Author: nick
 */

#include <algorithm>
#include "CRXRandom.h"

#include "omp.h"

#include "LocalPlace.h"
#include "CRXPerson.h"


namespace crx {

LocalPlace::LocalPlace(const PlaceParameters& params, const std::vector<int>& codes, std::shared_ptr<DosingParameters>& dosing_params) :
        CRXPlace(params), persons(), service_codes(codes), dosing_params_(dosing_params) {}

LocalPlace::~LocalPlace() {
}

void LocalPlace::reset() {
    persons.clear();
}

void LocalPlace::addPerson(const std::shared_ptr<CRXPerson>& person, int act_type) {
    persons.push_back(person);
    Statistics::instance()->initDosingEventRecording(person->id(), id_.place_id, DosingCause::USE);
    // dosing from use
    person->dose(this, dosing_params_->resource(), dosing_params_->newPlace(), Source::OTHER);
}

void LocalPlace::run(PropensityMap& propensity_map) {
    int thread_num = omp_get_thread_num();
    float decay = dosing_params_->decay();
    std::shuffle(persons.begin(), persons.end(), (*CRXRandom::instance()->engine(thread_num).get()));
    for (auto& p : persons) {
        // currently unknown places will be "parked" and added during decay
        // so this is exchanging info from t - 1.
        p->exchangeInfo(propensity_map, persons, dosing_params_->peer(), dosing_params_->newPlace());
        p->decay(decay);
    }
}

CRXPlace::const_iterator LocalPlace::serviceCodesBegin() {
    return service_codes.begin();
}

CRXPlace::const_iterator LocalPlace::serviceCodesEnd() {
    return service_codes.end();
}

size_t LocalPlace::size() const {
    return persons.size();
}

} /* namespace crx */
