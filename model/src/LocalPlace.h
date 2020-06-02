/*
 * LocalPlace.h
 *
 *  Created on: Oct 24, 2017
 *      Author: nick
 */

#ifndef LOCALPLACE_H_
#define LOCALPLACE_H_

#include <vector>

#include "CRXPlace.h"
#include "DosingParameters.h"

namespace crx {

class CRXPerson;

class LocalPlace: public CRXPlace {

protected:
    std::vector<std::shared_ptr<CRXPerson>> persons;
    std::vector<int> service_codes;
    std::shared_ptr<DosingParameters> dosing_params_;

public:

    LocalPlace(const PlaceParameters& params, const std::vector<int>& codes,
            std::shared_ptr<DosingParameters>& dosing_params);
    virtual ~LocalPlace();

    virtual void reset() override;
    virtual void addPerson(const std::shared_ptr<CRXPerson>& person, int act_type) override;

    /**
     * Runs this place executing its behavior (i.e. running the info transfer between persons in the place, etc.
     */
    void run(PropensityMap& propensity_map) override;

    size_t size() {
        return persons.size();
    }

    const_iterator serviceCodesBegin() override;
    const_iterator serviceCodesEnd() override;

    virtual size_t size() const override;
};

} /* namespace crx */

#endif /* LOCALPLACE_H_ */
