/*
 * HRXProvider.h
 *
 *  Created on: Feb 5, 2018
 *      Author: nick
 */

#ifndef HRXCACHE_H_
#define HRXCACHE_H_

#include <vector>
#include <memory>

#include "CRXPerson.h"

namespace crx {

class HRXCache {
    
private:
    std::map<unsigned int, std::vector<CRXPlace*>> cache;

public:
    HRXCache();
    virtual ~HRXCache();

    /**
     * Gets the places from the healthRx for this person.
     */
    std::vector<CRXPlace*>& getPlaces(std::shared_ptr<CRXPerson> person);

    void putPlace(unsigned int person_id, std::shared_ptr<CRXPlace> place);
    void clear();
};

} /* namespace crx */

#endif /* HRXCACHE_H_ */
