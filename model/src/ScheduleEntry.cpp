/*
 * Activity.cpp
 *
 *  Created on: Jul 20, 2017
 *      Author: nick
 */

#include "ScheduleEntry.h"

namespace crx {

Activity::Activity(unsigned int atus_code, DecisionType dt, float gamma) :  atus_code_{atus_code},
        decision_type{dt}, gamma_{gamma} {}

ScheduleEntry::ScheduleEntry(unsigned int schedule_id, const std::string& alpha_service_code, int service_code, const std::shared_ptr<Activity>& activity,
        float start_time, float end_time) :
        schedule_id_{schedule_id}, service_code_{service_code}, alpha_service_code_{alpha_service_code}, act_{activity}, start_{start_time}, end_{end_time} {}


bool ScheduleEntry::contains(float time) const {
    return start_ <= time && time < end_;
}


} /* namespace crx */
