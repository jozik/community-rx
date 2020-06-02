/*
 * CRXModel.h
 *
 *  Created on: Jul 22, 2016
 *      Author: nick
 */

#ifndef CRXMODEL_H_
#define CRXMODEL_H_

#include "chi_sim/AbstractModel.h"

#include "Network.h"
#include "CRXPerson.h"
#include "typedefs.h"
#include "Places.h"
#include "utils.h"
#include "ActivitiesCache.h"
#include "ServiceCodeMap.h"
#include "Clinic.h"
#include "PropensityMap.h"

namespace crx {

using AbsModelT = chi_sim::AbstractModel<CRXPerson, CRXPlace, CRXPersonData>;

class CRXModel: public AbsModelT {

private:
  chi_sim::Calendar cal;
  ActivitiesCache acts_cache;
  PropensityMap propensity_map;

  void initializePopulation(ServiceCodeMap& code_map, std::map<int, ClinicData>& clinics);
  void initializeStats(repast::ScheduleRunner& runner, ServiceCodeMap& map, std::map<int, ClinicData>& clinics);

protected:

  void nextActSelected(PersonPtr& person, chi_sim::NextPlace<CRXPlace>& next_act) override;


  /**
   * Create a person from the specified parameters.
   *
   * @param index the index of the start of the block of data for this Person
   */
  PersonPtr createPerson(unsigned int p_id, CRXPersonData& data) override;

  /**
   * Update the state of the specified person with the data in data.
   *
   * @param index the index of the start of the block of data for this Person
   */
  virtual void updatePerson(PersonPtr& person, CRXPersonData& data) override;

public:
  CRXModel(repast::Properties& props, MPI_Datatype person_data_type);

  virtual ~CRXModel();

  void step();

  void resetActivitySchedules();

  void writeData();

  void logBetaScores();

  void logHRXTagCounts();
};

} /* namespace crx */

#endif /* CRXMODEL_H_ */
