/*
 * HRXProvider.cpp
 *
 *  Created on: Feb 5, 2018
 *      Author: nick
 */

#include "HRXCache.h"

namespace crx {

HRXCache::HRXCache() : cache() {
}

HRXCache::~HRXCache() {
    cache.clear();
}

void HRXCache::clear() {
    cache.clear();
}


bool flagged = false;

std::vector<CRXPlace*>& HRXCache::getPlaces(std::shared_ptr<CRXPerson> person) {
    /*
    if (cache.size() == 0) {
        if (!flagged) {
            std::cout << "HRX RETURNING EMPTY VECTOR" << std::endl;
            flagged = true;
        }
        return empty_vec;
    }
    */
    return cache.at(person->id());
    //return cache[person->id()];
}

void HRXCache::putPlace(unsigned int person_id, std::shared_ptr<CRXPlace> place) {
    auto iter = cache.find(person_id);
    if (iter == cache.end()) {
        cache.emplace(person_id, std::vector<CRXPlace*>{place.get()});
    } else {
        iter->second.push_back(place.get());
    }
}

} /* namespace crx */
