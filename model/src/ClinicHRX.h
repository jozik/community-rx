/*
 * ClinicHRX.h
 *
 *  Created on: Feb 5, 2018
 *      Author: nick
 */

#ifndef CLINICHRX_H_
#define CLINICHRX_H_

#include "DosingParameters.h"
#include "HRXCache.h"
#include "CRXPerson.h"

namespace crx {

using DPMethodPtr = const float (DosingParameters::*)() const;

struct Doser {

    double prob;
    DPMethodPtr mp;
    int provider;

};

class ClinicHRX {
private:
    std::shared_ptr<HRXCache> hrx_cache;
    std::vector<Doser> dosers;

    const Doser& getDoser() const;

public:
    ClinicHRX(std::shared_ptr<HRXCache> provider, double dr_prob, double nurse_prob, double psr_prob);
    virtual ~ClinicHRX();

    void dosePerson(std::shared_ptr<DosingParameters>& dosing_params, int clinic_id, std::shared_ptr<CRXPerson> person) const;
};

} /* namespace crx */

#endif /* CLINICHRX_H_ */
