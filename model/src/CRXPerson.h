/*
 * CRXPerson.h
 *
 *  Created on: Jul 22, 2016
 *      Author: nick
 */

#ifndef CRXPERSON_H_
#define CRXPERSON_H_

#include "chi_sim/AbstractPerson.h"
#include "chi_sim/Calendar.h"

#include "typedefs.h"
#include "CRXPlace.h"
#include "Places.h"
#include "ActivitiesCache.h"
#include "crx_person_data.h"
#include "PropensityMap.h"
#include "place_source.h"

namespace crx {

using BasePersonT = chi_sim::AbstractPerson<CRXPlace, CRXPersonData>;
using PersonPtr = std::shared_ptr<CRXPerson>;

class CRXPerson : public BasePersonT {

private:
    ActsPtr weekday_acts_, weekend_acts_;
    Places places_;
    double alpha_;
    ActInfo current_act;
    bool switched_place;
    float info_propensity;
    unsigned int weekday_bucket_id, weekend_bucket_id;
    bool zonked;
    int age_id_;

public:
  CRXPerson(unsigned int num_id, const Places& places, double alpha, ActivitiesCache& cache, unsigned int weekday_bucket,
          unsigned int weekend_bucket, int age_id);

  virtual ~CRXPerson();

  const float infoPropensity() const {
      return info_propensity;
  }

  void update(std::map<int, std::shared_ptr<CRXPlace>>& place_map, ActivitiesCache& acts_cache, CRXPersonData& data);

  void fillSendData(CRXPersonData& data);

  void selectNextPlace(chi_sim::Calendar& cal, chi_sim::NextPlace<CRXPlace>& next_act);

  void dose(CRXPlace* place, float dose, float initial_val, Source source);

  double getBeta(int place_id);

  void logBetas(double tick, Statistics* stats);
  void logHRXTagCounts();

  void decay(float decay);

  void exchangeInfo(PropensityMap& propensity_map, std::vector<PersonPtr>& persons,
          float dose, float initial_beta);

  void resetWeekdayActivitySchedule(ActivitiesCache& cache);
  void resetWeekendActivitySchedule(ActivitiesCache& cache);

  void addInitialPlace(std::shared_ptr<CRXPlace>& place, double beta, bool is_initial_clinic = false);

  bool hasSwitchedPlace() const {
      return switched_place;
  }

  int currentServiceCode() const {
      return current_act.service_code;
  }

  int currentPlace() const {
      return current_act.place->placeId().place_id;
  }

};

} /* namespace crx */

#endif /* CRXPERSON_H_ */
