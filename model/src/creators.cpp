/*
 * creators.cpp

 *
 *  Created on: Jul 22, 2016
 *      Author: nick
 */


#include "repast_hpc/RepastProcess.h"
#include "chi_sim/CSVReader.h"


#include "creators.h"
#include "LocalPlace.h"
#include "Clinic.h"
#include "ClinicHRX.h"
#include "Places.h"

using namespace std;

namespace crx {

const int ID_IDX = 0;
const int PLACE_TYPE_IDX = 2;
const int LAT_IDX = 5;
const int LON_IDX = 6;
const int ZIP_IDX = 7;
const int PLACE_RANK_IDX = 8;

const int PERSON_RANK_IDX = 18;
const int HH_IDX = 11;
const int SCHOOL_IDX = 12;
const int WORK_IDX = 13;
const int WEEKDAY_ACTS_IDX = 15;
const int WEEKEND_ACTS_IDX = 16;
const int AGE_ID_IDX = 17;

const int ATUS_IDX = 0;
const int DECISION_TYPE_IDX = 1;
const int GAMMA_IDX = 2;

const int SCHEDULE_ID_IDX = 2;
const int START_IDX = 3;
const int END_IDX = 4;
const int SERVICE_CODE_IDX = 6;
// atus code
const int SCHEDULE_ACT_IDX = 5;

const int SERVICE_CODE_MAP_PID_IDX = 0;
const int SERVICE_CODE_MAP_ALPHA_CODE_IDX = 1;

const int ATUS_CODE_IDX = 0;
const int PROPENSITY_IDX = 1;
const int UNCERTAINTY_IDX = 2;

const int BUCKET_ID_IDX = 3;
const int BUCKET_AGE_ID = 1;
const int BUCKET_SCHEDULE_ID_IDX = 0;

const int CLINIC_ID_IDX = 0;
const int CLINIC_PROB_IDX = 2;
const int CLINIC_DOCTOR_IDX = 3;
const int CLINIC_NURSE_IDX = 4;
const int CLINIC_PSR_IDX = 5;

const int HRX_PERSON_IDX = 0;
const int HRX_PLACE_IDX = 1;

const int ALPHA_PID_IDX = 0;
const int ALPHA_ALPHA_IDX = 1;

const double EPSILON = 0.00001;


CRXPersonCreatorHelper::CRXPersonCreatorHelper() {
}

PersonPtr CRXPersonCreatorHelper::createLocalPerson(unsigned int num_id, const Places& places, double alpha,
        ActivitiesCache& cache, unsigned int weekday_bucket, unsigned int weekend_bucket, int age_id,
        std::vector<std::string>& row) {
    PersonPtr person = make_shared<CRXPerson>(num_id, places, alpha, cache, weekday_bucket, weekend_bucket, age_id);
    return person;
}

void CRXPersonCreatorHelper::processRemotePerson(unsigned int pid, int rank) {
    // turned off for now until we need it
    //network_->addNode(pid, rank, false);
}


void fill_hrx_cache(const std::string& hrx_cache_file, std::shared_ptr<HRXCache> hrx_cache, std::map<int, PlacePtr>& place_map) {
    chi_sim::CSVReader reader(hrx_cache_file);
    std::vector<std::string> line;

    // read the header
    reader.next(line);
    if (line.size() != 3) {
        throw invalid_argument(
                "HealtheRx input '" + hrx_cache_file + "' has invalid number of columns");
    }

    while (reader.next(line)) {
        unsigned int person_id = (unsigned int)stod(line[HRX_PERSON_IDX]);
        int place_id = (int)stod(line[HRX_PLACE_IDX]);
        auto iter = place_map.find(place_id);
        if (iter == place_map.end()) {
            throw domain_error("In HealtheRx Input place id " + to_string(place_id) + " not found in map of places.");
        }
        hrx_cache->putPlace(person_id, iter->second);
    }
}

void add_service_codes(const std::string& service_code_file, ServiceCodeMap& code_map) {
    chi_sim::CSVReader reader(service_code_file);
    std::vector<std::string> line;

    // read the header
    reader.next(line);
    if (line.size() != 1) {
        throw invalid_argument("Service code input '" + service_code_file + "' has invalid number of columns");
    }

    while (reader.next(line)) {
        std::string alpha_code = line[0];
        code_map.addServiceCode(alpha_code);
    }
}

void add_places_codes(const std::string& places_codes, ServiceCodeMap& code_map) {
    chi_sim::CSVReader reader(places_codes);
    std::vector<std::string> line;

    // read the header
    reader.next(line);
    if (line.size() != 2) {
        throw invalid_argument(
                "Places to service code input '" + places_codes
                        + "' has invalid number of columns");
    }

    while (reader.next(line)) {
        unsigned int pid = (unsigned int)stod(line[SERVICE_CODE_MAP_PID_IDX]);
        std::string alpha_code = line[SERVICE_CODE_MAP_ALPHA_CODE_IDX];
        code_map.addServiceCode(pid, alpha_code);
    }
}

void create_service_code_map(const std::string& service_code_file, const std::string& places_codes, ServiceCodeMap& code_map) {
    code_map.reset();
    add_service_codes(service_code_file, code_map);
    add_places_codes(places_codes, code_map);

    //std::cout << "G05: " << code_map.getNumericServiceCode("G05");
    //std::cout << ", G12: " << code_map.getNumericServiceCode("G12");
    //std::cout << ", G14: " << code_map.getNumericServiceCode("G14") << std::endl;
}

void create_clinic_map(const std::string& clinic_file, std::map<int, ClinicData>& clinic_data) {
    // numeric_id,case_id,hrx_probability,doctor_probability,nurse_probability,psr_probability
    chi_sim::CSVReader reader(clinic_file);
    std::vector<std::string> line;

    // read the header
    reader.next(line);
    if (line.size() != 6) {
        throw invalid_argument("Places input '" + clinic_file + "' has invalid number of columns");
    }

    while (reader.next(line)) {
        int id = (int)stod(line[CLINIC_ID_IDX]);
        double prob = stod(line[CLINIC_PROB_IDX]);
        double drp = stod(line[CLINIC_DOCTOR_IDX]);
        double nursep = stod(line[CLINIC_NURSE_IDX]);
        double psrp = stod(line[CLINIC_PSR_IDX]);

        clinic_data.emplace(id, ClinicData{prob, drp, nursep, psrp});
    }
}

void create_places(const std::string& place_file, std::map<int, PlacePtr>& place_map, std::map<int, ClinicData>& clinic_data,
        const ServiceCodeMap& code_map, std::shared_ptr<DosingParameters>& dosing_params,  std::shared_ptr<HRXCache> provider) {

    chi_sim::CSVReader reader(place_file);
    std::vector<std::string> line;

    // read the header
    reader.next(line);
    if (line.size() != 9) {
        throw invalid_argument("Places input '" + place_file + "' has invalid number of columns");
    }

    int my_rank = repast::RepastProcess::instance()->rank();
    int world_size = repast::RepastProcess::instance()->worldSize();

    PlaceParameters::Builder builder;

    while (reader.next(line)) {
        // for stod first for scientific notation
        int id = int(stod(line[ID_IDX]));
        std::string type(line[PLACE_TYPE_IDX]);
        GeoPoint p(stod(line[LAT_IDX]), stod(line[LON_IDX]));
        std::transform(type.begin(), type.end(), type.begin(), ::tolower);

        int zip = (int)stod(line[ZIP_IDX]);
        int rank = (int)stod(line[PLACE_RANK_IDX]);

        std::shared_ptr<CRXPlace> place;

        builder = builder.id(id).type(type).location(p).zipcode(zip).
                                        rank(rank);

        if (rank == my_rank || world_size == 1) {
            //std::cout << "rank: " << my_rank << " making local place " << id << std::endl;
            auto iter = clinic_data.find(id);
            if (iter == clinic_data.end()) {
                place = std::make_shared<LocalPlace>(builder.build(), code_map.getServiceCodes(id), dosing_params);
            } else {
                ClinicData& data = iter->second;
                if (data.hrx_p > 0 && abs(1 - (data.dr_p + data.nurse_p + data.psr_p)) > EPSILON) {
                    throw std::invalid_argument("Clinic provider probabilities must sum to 1");
                }

                ClinicHRX crx(provider, data.dr_p, data.nurse_p, data.psr_p);
                builder.type(CLINIC_TYPE);
                place = std::make_shared<Clinic>(builder.build(), code_map.getServiceCodes(id), dosing_params, data.hrx_p, crx);
            }
        } else {
            //std::cout << "rank: " << my_rank << " making non-local place " << id << " for rank " << rank << std::endl;
            auto iter = clinic_data.find(id);
            if (iter != clinic_data.end()) {
                builder.type(CLINIC_TYPE);
            }
            place = std::make_shared<CRXPlace>(builder.build());
        }

        place_map.emplace(id, place);
    }
}


std::shared_ptr<CRXPlace> find_place(const std::string& id, std::map<int, std::shared_ptr<CRXPlace>>& place_map) {
    if (id.length() > 0) {
        int num_id = (int)std::stod(id);
        auto iter = place_map.find(num_id);
        if (iter != place_map.end()) {
            return iter->second;
        }
    }

    return NULL_PLACE;
}

void assign_init_places(const std::string& init_places_file,
        std::map<unsigned int, std::shared_ptr<CRXPerson>>& person_map,
        std::map<int, PlacePtr>& place_map) {
    chi_sim::CSVReader reader(init_places_file);
    std::vector<std::string> line;

    // read the header
    reader.next(line);
    if (line.size() != 4) {
        throw invalid_argument(
                "Bad initial places input '" + init_places_file
                        + "' has invalid number of columns");
    }

    std::map<unsigned int, std::vector<std::pair<int, double>>> init_places_map;
    while (reader.next(line)) {
        unsigned int p_id = (unsigned int)stod(line[0]);
        int place_id = (int)stod(line[1]);
        double beta = stod(line[3]);

        std::vector<std::pair<int, double>>& vec = init_places_map[p_id];
        vec.push_back(std::pair<int, double>(place_id, beta));
    }

    for (auto& kv : person_map) {
        std::shared_ptr<CRXPerson>& person = kv.second;

        for (auto item : init_places_map.at(person->id())) {
            PlacePtr place = place_map.at(item.first);
            person->addInitialPlace(place, item.second);
        }
    }
}

void assign_init_clinics(const std::string& init_clinics_file, std::map<unsigned int, std::shared_ptr<CRXPerson>>& person_map,
        std::map<int, PlacePtr>& place_map) {
    chi_sim::CSVReader reader(init_clinics_file);
    std::vector<std::string> line;

    // read the header
    reader.next(line);
    if (line.size() != 3) {
       throw invalid_argument("Bad initial clinics input '" + init_clinics_file + "' has invalid number of columns");
    }

    std::map<unsigned int, std::vector<std::pair<int, double>>> clinic_map;
    while (reader.next(line)) {
        unsigned int p_id = (unsigned int)stod(line[0]);
        int place_id = (int)stod(line[1]);
        double beta = stod(line[2]);

        std::vector<std::pair<int, double>>& vec = clinic_map[p_id];
        vec.push_back(std::pair<int, double>(place_id, beta));
    }

    for (auto& kv : person_map) {
        std::shared_ptr<CRXPerson>& person = kv.second;

        for (auto item : clinic_map.at(person->id())) {
            PlacePtr clinic = place_map.at(item.first);
            if (clinic->type() != CLINIC_TYPE) {
                throw invalid_argument("Place '" + to_string(item.first) + "' is tagged as an initial clinic but is not marked as a clinic in clinic input");
            }
            person->addInitialPlace(clinic, item.second, true);
        }
    }
}


void assign_initial_places(const std::string& init_places_file,
        const std::string& init_clinic_file,
        std::map<unsigned int, std::shared_ptr<CRXPerson>>& person_map,
        std::map<int, PlacePtr>& place_map) {

    // import to add the clinics first so the clinic beta value
    // takes precedence
    assign_init_clinics(init_clinic_file, person_map, place_map);
    assign_init_places(init_places_file, person_map, place_map);
}

void create_person_alpha_map(const std::string& file, std::map<unsigned int, double>& alpha_map) {
    chi_sim::CSVReader reader(file);
     std::vector<std::string> line;

    // read the header
    reader.next(line);
    if (line.size() != 2) {
        throw invalid_argument("Alpha input '" + file + "' has invalid number of columns");
    }

    while (reader.next(line)) {
        unsigned int id = (unsigned int)stod(line[ALPHA_PID_IDX]);
        double alpha = std::stod(line[ALPHA_ALPHA_IDX]);
        alpha_map.emplace(id, alpha);
    }
}


void create_persons(const std::string& person_file,
        const std::string& person_alpha_file,
        std::map<unsigned int, std::shared_ptr<CRXPerson>>& person_map,
        std::map<int, PlacePtr>& place_map,
        ActivitiesCache& acts_cache,
        const ServiceCodeMap& code_map,
        CRXPersonCreatorHelper& helper) {

    std::map<unsigned int, double> alpha_map;
    create_person_alpha_map(person_alpha_file, alpha_map);

    chi_sim::CSVReader reader(person_file);
    std::vector<std::string> line;

    // read the header
    reader.next(line);
    if (line.size() != 19) {
        throw invalid_argument("Person input '" + person_file + "' has invalid number of columns");
    }

    int my_rank = repast::RepastProcess::instance()->rank();
    int world_size = repast::RepastProcess::instance()->worldSize();

    // TODO get beta from file when data exists
    double beta = INIT_BETA;
    while (reader.next(line)) {
        unsigned int id = (unsigned int)stod(line[ID_IDX]);
        int p_rank = (int)std::stod(line[PERSON_RANK_IDX]);
        if (p_rank == my_rank || world_size == 1) {
            Places places;
            std::shared_ptr<CRXPlace> p = find_place(line[WORK_IDX], place_map);
            if (p->placeId().exists()) places.addPlace(p, beta, true);
            p = find_place(line[SCHOOL_IDX], place_map);
            if (p->placeId().exists())  places.addPlace(p, beta, true);
            p = find_place(line[HH_IDX], place_map);
            if (!p->placeId().exists()) {
                std::cout << "Missing HH: (" << std::stoi(line[HH_IDX]) << ")" << std::endl;
                throw invalid_argument("");
            }
            if (p->placeId().exists())  places.addPlace(p, beta, true);

            unsigned int weekday_bucket_id = (unsigned int)stod(line[WEEKDAY_ACTS_IDX]);
            unsigned int weekend_bucket_id = (unsigned int)stod(line[WEEKEND_ACTS_IDX]);
            int age_id = (int)stod(line[AGE_ID_IDX]);
            double alpha = alpha_map.at(id);

            PersonPtr person  = helper.createLocalPerson(id, places, alpha, acts_cache, weekday_bucket_id, weekend_bucket_id, age_id, line);
            person_map.emplace(id, person);
        } else {
            helper.processRemotePerson(id, p_rank);
        }
    }
}

void read_acts_file(const std::string& file, std::map<unsigned int, std::shared_ptr<Activity>>& act_map,
        std::map<std::string, float>& gamma_map) {
   chi_sim::CSVReader reader(file);
   std::vector<std::string> line;

   // read the header
   reader.next(line);
   if (line.size() != 3) {
       throw invalid_argument("Activities input '" + file + "' has invalid number of columns");
   }

   while (reader.next(line)) {
       unsigned int atus = (unsigned int)stod(line[ATUS_IDX]);
       std::string str_gamma = line[GAMMA_IDX];
       auto iter = gamma_map.find(str_gamma);
       if (iter == gamma_map.end()) {
           throw invalid_argument("Activities input '" + file + "' contains bad gamma value: " + str_gamma);
       }
       DecisionType dt = stoi(line[DECISION_TYPE_IDX]) == 0 ? DecisionType::AB : DecisionType::A;
       act_map.emplace(atus, std::make_shared<Activity>(atus, dt, iter->second));
   }
}

void read_schedule_file(const std::string& schedule_file, std::map<unsigned int, ActsPtr>& schedule_map,
        std::map<unsigned int, std::shared_ptr<Activity>>& act_map,  const ServiceCodeMap& code_map) {

    chi_sim::CSVReader reader(schedule_file);
    std::vector<std::string> line;

    // read the header
    reader.next(line);
    if (line.size() != 7) {
        throw invalid_argument("Schedule input '" + schedule_file + "' has invalid number of columns");
    }

    // header is line 1
    unsigned int line_idx = 2;
    while (reader.next(line)) {
        // make individual entry
        unsigned int id =  (unsigned int)stod(line[SCHEDULE_ID_IDX]);
        // schedule times are seconds from 00:00:00
        double start = stod(line[START_IDX]) / 60;
        double end = stod(line[END_IDX]) / 60;
        // convert end of 0 to 24:00
        if (end == 0) {
            end = 86400 / 60;
        }
        std::string alpha_service_code = repast::trim(line[SERVICE_CODE_IDX]);
        unsigned int atus_code = (unsigned int)stod(line[SCHEDULE_ACT_IDX]);

        int service_code = code_map.getNumericServiceCode(alpha_service_code);
        // default null act
        std::shared_ptr<Activity> act;
        auto act_iter = act_map.find(atus_code);
        if (act_iter == act_map.end()) {
            if (alpha_service_code != ZONKED_SERVICE_CODE_ALPHA) {
                throw invalid_argument("Error creating activities. atus code " + line[SCHEDULE_ACT_IDX] + " not found on line " + std::to_string(line_idx));
            }
        } else {
            act = act_iter->second;
        }

        std::shared_ptr<ScheduleEntry> entry = std::make_shared<ScheduleEntry>(id, alpha_service_code, service_code, act, start, end);

        auto iter = schedule_map.find(entry->scheduleId());
        if (iter == schedule_map.end()) {
            std::shared_ptr<ActsT> acts = std::make_shared<ActsT>(id);
            acts->addActivity(entry);
            schedule_map.emplace(id, acts);
        } else {
            iter->second->addActivity(entry);
        }
        ++line_idx;
    }
}

void read_buckets_file(const std::string buckets_file, ActivitiesCache& acts_cache,  std::map<unsigned int, ActsPtr>& schedule_map) {
    chi_sim::CSVReader reader(buckets_file);
    std::vector<std::string> line;


    // read the header
    reader.next(line);

    // header is line 1
    unsigned int line_idx = 2;
    if (line.size() != 4) {
        throw invalid_argument("Schedule buckets file  '" + buckets_file + "' has invalid number of columns");
    }

    while (reader.next(line)) {
        unsigned int bucket_id =  (unsigned int)stod(line[BUCKET_ID_IDX]);
        unsigned int schedule_id = (unsigned int)stod(line[BUCKET_SCHEDULE_ID_IDX]);
        int age_id = (int)stod(line[BUCKET_AGE_ID]);
        auto iter = schedule_map.find(schedule_id);
        if (iter == schedule_map.end()) {
            throw invalid_argument("Error creating schedule buckets. Schedule with id " + line[BUCKET_SCHEDULE_ID_IDX] + " not found - line " + std::to_string(line_idx));
        }
        acts_cache.addActivities(bucket_id, age_id, iter->second);
        ++line_idx;
    }
}

void create_activities(const std::string& schedule_file, const std::string& acts_file, const std::string& buckets_file, const ServiceCodeMap& code_map,
        ActivitiesCache& acts_cache, std::map<std::string, float>& gamma_map) {

    std::map<unsigned int, std::shared_ptr<Activity>> act_map;
    read_acts_file(acts_file, act_map, gamma_map);

    std::map<unsigned int, ActsPtr> schedule_map;
    read_schedule_file(schedule_file, schedule_map, act_map, code_map);

    read_buckets_file(buckets_file, acts_cache, schedule_map);

}

void create_propensities(const std::string& propensity_file, PropensityMap& propensity_map, PropensityValues& propensity_values) {
    chi_sim::CSVReader reader(propensity_file);
    std::vector<std::string> line;

    // read the header
    reader.next(line);
    if (line.size() != 3) {
        throw invalid_argument("propensity input '" + propensity_file + "' has invalid number of columns");
    }

    std::map<std::string, VecOfNormalDistPtr> dist_map;
    while (reader.next(line)) {
        unsigned int atus_code = (unsigned int)stod(line[ATUS_CODE_IDX]);
        std::string prop_code = line[PROPENSITY_IDX];
        std::string unc_code = line[UNCERTAINTY_IDX];

        std::string code = prop_code + unc_code;
        auto iter = dist_map.find(code);
        if (iter == dist_map.end()) {
            float prop = propensity_values.getPropensity(prop_code);
            float unc = propensity_values.getUncertainty(unc_code);
            VecOfNormalDistPtr vec = std::make_shared<VecOfNormalDist>();
            CRXRandom::instance()->createNormalDistributions(prop, prop * unc, vec);
            dist_map.emplace(code, vec);
            propensity_map.addDistribution(atus_code, vec);
        } else {
            propensity_map.addDistribution(atus_code, iter->second);
        }
    }
}

float PropensityValues::getPropensity(const std::string& code) {
    if (code == "N")
        return n_propensity;
    if (code == "L")
        return l_propensity;
    if (code == "M")
        return m_propensity;
    if (code == "H")
        return h_propensity;

    throw invalid_argument("Invalid propensity value '" + code + "'. Must be one of N, L, M, or H");
}

float PropensityValues::getUncertainty(const std::string& code) {
    if (code == "L")
        return l_uncertainty;
    if (code == "M")
        return m_uncertainty;
    if (code == "H")
        return h_uncertainty;

    throw invalid_argument(
            "Invalid uncertainty value '" + code + "'. Must be one of L, M, or H");
}

}
