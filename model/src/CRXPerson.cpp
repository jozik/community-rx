/*
 * CRXPerson.cpp
 *
 *  Created on: Jul 22, 2016
 *      Author: nick
 */

#include <exception>

#include "omp.h"

#include "repast_hpc/RepastProcess.h"

#include "CRXPerson.h"
#include "constants.h"
#include "NetworkStatsRecorder.h"
#include "CRXRandom.h"


namespace crx {

CRXPerson::CRXPerson(unsigned int num_id, const Places& places, double alpha, ActivitiesCache& cache, unsigned int weekday_bucket,
        unsigned int weekend_bucket, int age_id) : BasePersonT(num_id), weekday_acts_{cache.selectSchedule(weekday_bucket, age_id)},
            weekend_acts_{cache.selectSchedule(weekend_bucket, age_id)}, places_{places}, alpha_(alpha), current_act{places_.home(), 10101}, switched_place{false},
            info_propensity(1), weekday_bucket_id{weekday_bucket}, weekend_bucket_id{weekend_bucket}, zonked(false), age_id_{age_id} {

}

CRXPerson::~CRXPerson() {
    places_.clear();
}

void CRXPerson::fillSendData(CRXPersonData& data) {
  BasePersonT::fillSendData(data);
  data.weekday_acts = weekday_acts_->id();
  data.weekend_acts = weekend_acts_->id();
  data.alpha = alpha_;
  data.act_info_place_id = current_act.place->placeId().place_id;
  data.act_info_atus_code = current_act.atus_code;
  data.act_info_service_code = current_act.service_code;
  data.switched_place = switched_place ? 1 : 0;
  data.info_propensity = info_propensity;
  data.weekday_bucket_id = weekday_bucket_id;
  data.weekend_bucket_id = weekend_bucket_id;
  data.zonked = zonked ? 1 : 0;
  data.age_id = age_id_;

  
  std::fill(data.places_betas, data.places_betas + MAX_PLACES, (double)PLACE_FILLER);
  std::fill(data.places_place_ids, data.places_place_ids + MAX_PLACES, PLACE_FILLER);
  places_.fillSendData(data);
}

void CRXPerson::update(std::map<int, std::shared_ptr<CRXPlace>>& place_map, ActivitiesCache& acts_cache, CRXPersonData& data) {
   weekday_acts_ = acts_cache.getSchedule(data.weekday_acts);
   weekend_acts_ = acts_cache.getSchedule(data.weekend_acts);

   current_act.atus_code = data.act_info_atus_code;
   current_act.service_code = data.act_info_service_code;
   current_act.place = place_map.at(data.act_info_place_id).get();

   switched_place = data.switched_place == 1;
   info_propensity = data.info_propensity;
   zonked = data.zonked == 1;

   places_.reset(place_map, data, MAX_PLACES);
}

void CRXPerson::resetWeekdayActivitySchedule(ActivitiesCache& cache) {
    weekday_acts_ = cache.selectSchedule(weekday_bucket_id, age_id_);
}

void CRXPerson::resetWeekendActivitySchedule(ActivitiesCache& cache) {
    weekend_acts_ = cache.selectSchedule(weekend_bucket_id, age_id_);
}

void CRXPerson::selectNextPlace(chi_sim::Calendar& calendar, chi_sim::NextPlace<CRXPlace>& next_act) {
    ActsPtr& acts = calendar.isWeekDay() ? weekday_acts_ : weekend_acts_;
    std::shared_ptr<ActT> entry = acts->findActivityAt(calendar.minute_of_day);

     if (entry == ActsT::NULL_ACT) {
         throw std::domain_error("Activity schedule " + std::to_string(acts->id()) +
                 " is missing activity for " + std::to_string(calendar.minute_of_day));
     } else {
         Statistics::instance()->setPersonId(id_);
         int service_code = entry->serviceCode();
         zonked = service_code == ZONKED_SERVICE_CODE;
         int current_place = current_act.place->placeId().place_id;
         unsigned int current_atus_code = current_act.atus_code;

         if (!zonked) {
             places_.selectPlace(entry, alpha_, current_act);
         } else {
             // stay where you are but do the atus_code
             current_act.atus_code = entry->act()->atus_code_;
             current_act.service_code = service_code;
         }

         next_act.id = current_act.place->placeId();
         next_act.place = current_act.place;
         // CRX model doesn't use this
         next_act.act_type = 0;
         switched_place = next_act.id.place_id != current_place;
        if (switched_place) {
            PlaceEdgeRecorder::instance()->recordEdge(current_place, next_act.id.place_id, id_);
        }
         // if (id_ == 1207133) {
         //   std::cout << calendar.minute_of_day << ": " << entry->start() << ", " << entry->end() << ", " << entry->alphaServiceCode() << ", " << entry->act()->atus_code_ << std::endl;
         //   double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();
         //   std::cout << tick << ", " << entry->scheduleId() << "," << service_code  << ", " << entry->alphaServiceCode() << ", " <<
         //           current_act.atus_code << ", " << current_place << ", " << next_act.id.place_id << ", " << success << std::endl;
         // }

         if (switched_place || current_act.atus_code != current_atus_code) {
             // only record visit if place changed, otherwise actually in same place spanning 1+ time steps
             Statistics::instance()->recordVisit(next_act.id.place_id, service_code, entry->alphaServiceCode(), current_act.atus_code, entry->scheduleId());
         }
     }
}

double CRXPerson::getBeta(int place_id) {
    return places_.getBeta(place_id);
}

void CRXPerson::dose(CRXPlace* place, float dose, float initial_val, Source source) {
    places_.dose(place, dose, initial_val, source);
}

void CRXPerson::decay(float decay) {
    // stats
    Statistics::instance()->initDosingEventRecording(id(), current_act.place->placeId().place_id, DosingCause::DECAY);
    places_.decayBetas(decay);
}

void CRXPerson::logBetas(double tick, Statistics* stats) {
    places_.logBetas(tick, id_, stats);
}

void CRXPerson::logHRXTagCounts() {
    places_.logHRXTagCounts(id_);
}

void CRXPerson::addInitialPlace(std::shared_ptr<CRXPlace>& place, double beta, bool is_initial_clinic) {
    // the order is important here
    if (is_initial_clinic) {
        places_.setInitialClinic(place);
    }
    places_.addPlace(place, beta, true);
}

void CRXPerson::exchangeInfo(PropensityMap& propensity_map, std::vector<PersonPtr>& persons,
        float dose, float initial_beta) {
    //std::cout << "atus_code: " << current_act.atus_code << std::endl;
    if (!zonked) {
        NetworkStatsRecorder::instance()->initInfoNetworkRecordingFor(id());

        size_t n = persons.size();
        if ( n > 1) {

            boost::container::flat_set<int> added_places;
            int thread_num = omp_get_thread_num();
            double threshold = propensity_map.next(current_act.atus_code, thread_num);

            Statistics::instance()->initDosingEventRecording(id(), current_act.place->placeId().place_id, DosingCause::PEER, 
                threshold, persons.size());

            for (size_t i = 0; i < n; ++i) {
                auto& p = persons[i];
                double draw = p->infoPropensity() * CRXRandom::instance()->next(thread_num);
                //std::cout << p->infoPropensity() << ", " << draw << ", " << threshold;
                if (p.get() != this && !p->zonked && draw <= threshold) {
                    NetworkStatsRecorder::instance()->addInfoNetworkLink(p->id());
                    Statistics::instance()->initHRXSharedEventRecording(p->id(), id(), current_act.place->placeId().place_id);
                    p->places_.doseShared(added_places, places_, dose, initial_beta); //vec[thread_num]);
                }
            }
            NetworkStatsRecorder::instance()->finalizeInfoNetworkRecording();
        }
    }
}

} /* namespace crx */
