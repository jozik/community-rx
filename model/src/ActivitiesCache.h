/*
 * ActivitiesCache.h
 *
 *  Created on: Jan 15, 2018
 *      Author: nick
 */

#ifndef ACTIVITIESCACHE_H_
#define ACTIVITIESCACHE_H_

#include <map>
#include <vector>

#include "typedefs.h"

namespace crx {

class ActivitiesCache {
private:
    std::map<unsigned int, ActsPtr> acts_map;
    // bucket_id -> k: age_id, v: vector of act_ids
    std::map<unsigned int, std::map<int, std::vector<unsigned int>>> buckets;

public:
    ActivitiesCache();
    virtual ~ActivitiesCache();

    void addActivities(unsigned int bucket_id, int age_id, ActsPtr acts);

    ActsPtr selectSchedule(unsigned int bucket_id, int age_id);
    ActsPtr getSchedule(unsigned int schedule_id);

    size_t actsSize() {
        return acts_map.size();
    }

    size_t bucketsSize() {
        return buckets.size();
    }

    void clear();

};

} /* namespace crx */

#endif /* ACTIVITIESCACHE_H_ */
