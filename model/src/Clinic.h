/*
 * Clinic.h
 *
 *  Created on: Jan 30, 2018
 *      Author: nick
 */

#ifndef CLINIC_H_
#define CLINIC_H_

#include "LocalPlace.h"
#include "ClinicHRX.h"
#include "HRXCache.h"

namespace crx {

// Holds clinic data when read from input file
struct ClinicData {
    double hrx_p, dr_p, nurse_p, psr_p;
};

class Clinic: public LocalPlace {

private:
    double crx_prob;
    ClinicHRX clinic_hrx_;

public:
    Clinic(const PlaceParameters& params, const std::vector<int>& codes,
            std::shared_ptr<DosingParameters>& dosing_params, double crx_probability,
            ClinicHRX& clinic_hrx);
    virtual ~Clinic();

    /**
     * Runs this place executing its behavior (i.e. running the info transfer between persons in the place, etc.
     */
    void run(PropensityMap& propensity_map) override;

};

} /* namespace crx */

#endif /* CLINIC_H_ */
