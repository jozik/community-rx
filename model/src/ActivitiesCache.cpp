/*
 * ActivitiesCache.cpp
 *
 *  Created on: Jan 15, 2018
 *      Author: nick
 */

#include "repast_hpc/Random.h"

#include "ActivitiesCache.h"

namespace crx {

ActivitiesCache::ActivitiesCache() : acts_map(), buckets() {
}

ActivitiesCache::~ActivitiesCache() {
    acts_map.clear();
    buckets.clear();
}

void ActivitiesCache::addActivities(unsigned int bucket_id, int age_id, ActsPtr acts) {
    acts_map.emplace(acts->id(), acts);
    auto iter = buckets.find(bucket_id);
    if (iter == buckets.end()) {
        buckets.emplace(bucket_id, std::map<int, std::vector<unsigned int>>{{age_id, std::vector<unsigned int>{acts->id()}}});
    } else {
        iter->second[age_id].push_back(acts->id());
    }
}

ActsPtr ActivitiesCache::getSchedule(unsigned int schedule_id) {
    return acts_map.at(schedule_id);
}

ActsPtr ActivitiesCache::selectSchedule(unsigned int bucket_id, int age_id) {
    std::map<int, std::vector<unsigned int>>& m = buckets[bucket_id];
    if (m.size() == 0) {
        throw std::invalid_argument("ActivitiesCache.selectSchedule: bad bucket_id " + std::to_string(bucket_id));
    }

    // some buckets only have age_id of 0.
    std::vector<unsigned int>& ids = m.size() == 1 ? m[0] : m[age_id];

    size_t index = (int)repast::Random::instance()->createUniIntGenerator(0, ids.size() - 1).next();
    return acts_map[ids[index]];
}

void ActivitiesCache::clear() {
    acts_map.clear();
    buckets.clear();
}

} /* namespace crx */
