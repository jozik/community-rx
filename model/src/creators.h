/*
 * creators.h
 *
 *  Created on: Jul 22, 2016
 *      Author: nick
 */

#ifndef CREATORS_H_
#define CREATORS_H_


#include "typedefs.h"
#include "CRXPerson.h"
#include "CRXPlace.h"
#include "Network.h"
#include "ServiceCodeMap.h"
#include "Places.h"
#include "DosingParameters.h"
#include "utils.h"
#include "ActivitiesCache.h"
#include "ClinicHRX.h"
#include "Clinic.h"
#include "HRXCache.h"
#include "PropensityMap.h"

namespace crx {

struct PropensityValues {
    float n_propensity, l_propensity, m_propensity, h_propensity;
    float l_uncertainty, m_uncertainty, h_uncertainty;

    float getPropensity(const std::string& prop_code);
    float getUncertainty(const std::string& prop_code);

};

struct CRXPersonCreatorHelper {

    CRXPersonCreatorHelper();

    PersonPtr createLocalPerson(unsigned int num_id, const Places& places, double alpha,
            ActivitiesCache& cache, unsigned int weekday_bucket, unsigned int weekend_bucket, int age_id,
            std::vector<std::string>& row);

    void processRemotePerson(unsigned int pid, int rank);
};

void fill_hrx_cache(const std::string& hrx_cache_file, std::shared_ptr<HRXCache> hrx_cache, std::map<int, PlacePtr>& place_map);

void create_clinic_map(const std::string& clinic_file, std::map<int, ClinicData>& clinic_data);

void create_service_code_map(const std::string& service_code_file, const std::string& places_services_code_file, ServiceCodeMap& code_map);

void create_places(const std::string& file, std::map<int, PlacePtr>& place_map, std::map<int, ClinicData>& clinic_data,
        const ServiceCodeMap& code_map, std::shared_ptr<DosingParameters>& dosing_params, std::shared_ptr<HRXCache> provider);

void create_activities(const std::string& schedule_file, const std::string& acts_file, const std::string& buckets_file, const ServiceCodeMap& code_map,
        ActivitiesCache& acts_cache, std::map<std::string, float>& gamma_map);

void assign_initial_places(const std::string& init_places_file,
        const std::string& init_clinic_file,
        std::map<unsigned int, std::shared_ptr<CRXPerson>>& person_map,
        std::map<int, PlacePtr>& place_map);

void create_persons(const std::string& file,
        const std::string& person_alpha_file,
        std::map<unsigned int, std::shared_ptr<CRXPerson>>& person_map,
        std::map<int, PlacePtr>& place_map,
        ActivitiesCache& acts_cache,
        const ServiceCodeMap& code_map,
        CRXPersonCreatorHelper& helper);

void create_propensities(const std::string& propensity_file, PropensityMap& propensity_map,
        PropensityValues& propensity_values);

}


#endif /* CREATORS_H_ */
