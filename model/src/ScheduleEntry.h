/*
 * Activity.h
 *
 *  Created on: Jul 20, 2017
 *      Author: nick
 */

#ifndef SCHEDULEENTRY_H_
#define SCHEDULEENTRY_H_

#include <memory>
#include <vector>

#include "chi_sim/PlaceId.h"

#include "CRXPlace.h"

namespace crx {

enum class DecisionType { A, AB};

struct Activity {

    unsigned int atus_code_;
    DecisionType decision_type;
    float gamma_;

    Activity(unsigned int atus_code, DecisionType dt, float gamma);

};

class ScheduleEntry {

private:
	unsigned int schedule_id_;
	int service_code_;
	std::string alpha_service_code_;
	std::shared_ptr<Activity> act_;
	float start_, end_;

public:
	// i, id, service_code, crx_type, act_type, start, end
    ScheduleEntry(unsigned int schedule_id_, const std::string& alpha_service_code, int service_code, const std::shared_ptr<Activity>& activity,
            float start_time, float end_time);
    ~ScheduleEntry(){}

    /**
     * Whether or not the specified time is within the time range of this activity.
     * Range is [start, end).
     */
    bool contains(float time) const;

   int serviceCode() {
        return service_code_;
    }

   const std::string& alphaServiceCode() const {
       return alpha_service_code_;
   }

    unsigned int scheduleId() const {
        return schedule_id_;
    }

    const std::shared_ptr<Activity> act() const {
        return act_;
    }

    float start() const {
       return start_;
   }

   float end() const {
       return end_;
   }

};

} /* namespace crx */

#endif /* SCHEDULEENTRY_H_ */
