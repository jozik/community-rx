/*
 * ClinicHRX.cpp
 *
 *  Created on: Feb 5, 2018
 *      Author: nick
 */

#include <exception>

#include "omp.h"

#include "repast_hpc/Random.h"

#include "ClinicHRX.h"
#include "CRXRandom.h"
#include "place_source.h"

namespace crx {

const double EPSILON = 0.00001;
const int DOCTOR = 0;
const int NURSE = 1;
const int PSR = 2;

ClinicHRX::ClinicHRX(std::shared_ptr<HRXCache> cache, double dr_prob, double nurse_prob, double psr_prob) : hrx_cache(cache),
        dosers() {

    dosers.push_back(Doser{dr_prob, &DosingParameters::doctor, DOCTOR});
    dosers.push_back(Doser{dr_prob + nurse_prob, &DosingParameters::nurse, NURSE});
    dosers.push_back(Doser{1.0, &DosingParameters::psr, PSR});
}

const Doser& ClinicHRX::getDoser() const {
    int thread_num = omp_get_thread_num();
    double draw = CRXRandom::instance()->next(thread_num);
    for (auto& doser : dosers) {
        if (draw <= doser.prob) {
            return doser;
            //return 
        }
    }
    throw std::domain_error("Error getting dosing parameter in ClinicHRX");
}

void ClinicHRX::dosePerson(std::shared_ptr<DosingParameters>& dosing_params, int clinic_id, std::shared_ptr<CRXPerson> person) const {
    Doser doser = getDoser();
    float dose = (dosing_params.get()->*doser.mp)();
    int provider = doser.provider;
    float new_place = dosing_params->newPlace();
    Statistics::instance()->recordHRXReceivedEvent(person->id(), clinic_id, provider);
    //std::cout << person->id() << std::endl;
    Statistics::instance()->initDosingEventRecording(person->id(), clinic_id, DosingCause::HRX);
    for (auto& place : hrx_cache->getPlaces(person)) {
        person->dose(place, dose, new_place, Source::HRX);
    }
}

ClinicHRX::~ClinicHRX() {
    //hrx_cache->clear();
}

} /* namespace crx */
