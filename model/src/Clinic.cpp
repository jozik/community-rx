/*
 * Clinic.cpp
 *
 *  Created on: Jan 30, 2018
 *      Author: nick
 */

#include "repast_hpc/Random.h"

#include "Clinic.h"
#include "constants.h"
#include "omp.h"

namespace crx {

bool is_clinic_service_code(int service_code) {
    return (service_code == CLINIC_CODE_1 || service_code == CLINIC_CODE_2 || service_code == CLINIC_CODE_3 || service_code == CLINIC_CODE_4 ||
        service_code == CLINIC_CODE_5);
}

Clinic::Clinic(const PlaceParameters& params, const std::vector<int>& codes,
        std::shared_ptr<DosingParameters>& dosing_params, double crx_probability, ClinicHRX& clinic_hrx) : LocalPlace(params, codes, dosing_params),
                crx_prob(crx_probability), clinic_hrx_(clinic_hrx) {
    type_ = CLINIC_TYPE;
}

Clinic::~Clinic() {
}


void Clinic::run(PropensityMap& propensity_map) {
    int thread_num = omp_get_thread_num();

    for (auto& person : persons) {
        // give hrx on first arrival rather than possibly every hour at clinic
        if (is_clinic_service_code(person->currentServiceCode()) && person->hasSwitchedPlace() && CRXRandom::instance()->next(thread_num) <= crx_prob) {
            // clinic provides healtheRx
            // std::cout << person->id() << std::endl;
            clinic_hrx_.dosePerson(dosing_params_, id_.place_id, person);
        }
    }

    LocalPlace::run(propensity_map);
}

} /* namespace crx */
