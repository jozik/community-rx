/*
 * CRXModel.cpp
 *
 *  Created on: Jul 22, 2016
 *      Author: nick
 */
#include<string>

#include "omp.h"

#include "boost/tokenizer.hpp"
#include "boost/algorithm/string.hpp"

#include "repast_hpc/Schedule.h"
#include "repast_hpc/io.h"

#include "chi_sim/Parameters.h"
#include "chi_sim/file_utils.h"
//#include "chi_sim/FileOutput.h"

#include "CRXModel.h"
#include "creators.h"
#include "parameter_constants.h"
#include "constants.h"
#include "PlaceScoreCalculator.h"

#include "LocalPlace.h"
#include "Statistics.h"
#include "Clinic.h"
#include "ResourceVisitRecorder.h"
#include "DosingRecorder.h"
#include "NetworkStatsRecorder.h"

using namespace chi_sim;
using namespace repast;

namespace crx {

template<typename T>
struct Converter {
    static T from_string(const std::string& val);
};

template<>
struct Converter<unsigned int> {
    static unsigned int from_string(const std::string& val) {
        return std::stol(val);
    }
};

template<>
struct Converter<int> {
    static int from_string(const std::string& val) {
        return std::stoi(val);
    }
};

template<>
struct Converter<std::string> {
    static std::string from_string(const std::string& val) {
        return val;
    }
};

template<>
struct Converter<DosingCause> {
    static DosingCause from_string(const std::string& val) {
        if (val == "HRX") return DosingCause::HRX;
        if (val == "PEER") return DosingCause::PEER;
        if (val == "USE") return DosingCause::USE;
        if (val == "DECAY") return DosingCause::DECAY;
        if (val == "REMOVE") return DosingCause::REMOVE;

        throw new std::invalid_argument("Bad dosing cause value in " + DOSING_RECORDING_CAUSES);
    }
};

template<typename T>
std::shared_ptr<Filter<T>> create_filter(const std::string& filter_string, std::map<int, ClinicData>& clinics) {
    std::shared_ptr<Filter<T>> filter;

    if (filter_string == "NONE") {
        filter = std::make_shared<NeverPassFilter<T>>();
    } else if (filter_string == "ALL") {
        filter = std::make_shared<AlwaysPassFilter<T>>();
    } else {
        std::shared_ptr<ContainsFilter<T>> cf = std::make_shared<ContainsFilter<T>>();
        boost::char_separator<char> sep(",");
        boost::tokenizer<boost::char_separator<char>> tok(filter_string, sep);
        for (auto item : tok) {
            boost::trim(item);
            if (item == "CLINICS") {
                for (auto& kv : clinics) {
                    // need to convert to and from to get the templates to resolve
                    // without error
                    cf->addItem(Converter<T>::from_string(std::to_string(kv.first)));
                }
            } else {
                cf->addItem(Converter<T>::from_string(item));
            }
        }
        filter = cf;
    }
    return filter;
}

void CRXModel::initializeStats(ScheduleRunner& runner, ServiceCodeMap& map, std::map<int, ClinicData>& clinics) {
    string output_directory =  Parameters::instance()->getStringParameter(OUTPUT_DIRECTORY);
    std::string choice_fname = output_directory + "/" + Parameters::instance()->getStringParameter(CHOICE_OUTPUT_FILE);
    std::string visits_fname =  output_directory + "/" + Parameters::instance()->getStringParameter(VISIT_OUTPUT_FILE);

    std::shared_ptr<Filter<unsigned int>> code_filter = create_filter<unsigned int>(Parameters::instance()->getStringParameter(VISIT_RECORDING_ATUS_CODES), clinics);
    std::shared_ptr<Filter<std::string>> service_filter = create_filter<std::string>(Parameters::instance()->getStringParameter(VISIT_RECORDING_SERVICE_CODES), clinics);
    std::shared_ptr<Filter<unsigned int>> person_filter = create_filter<unsigned int>(Parameters::instance()->getStringParameter(VISIT_RECORDING_PERSONS), clinics);
    std::shared_ptr<Filter<int>> places_filter = create_filter<int>(Parameters::instance()->getStringParameter(VISIT_RECORDING_PLACES), clinics);
    ResourceVisitRecorder visit_recorder(visits_fname, rank_, service_filter, code_filter, person_filter, places_filter);

    std::shared_ptr<Filter<unsigned int>> dosing_person_filter = create_filter<unsigned int>(Parameters::instance()->getStringParameter(DOSING_RECORDING_PERSONS), clinics);
    std::shared_ptr<Filter<DosingCause>> dosing_cause_filter = create_filter<DosingCause>(Parameters::instance()->getStringParameter(DOSING_RECORDING_CAUSES), clinics);
    std::string dosing_fname =  output_directory + "/" + Parameters::instance()->getStringParameter(DOSING_OUTPUT_FILE);
    DosingRecorder dosing_recorder(dosing_fname, rank_, dosing_person_filter, dosing_cause_filter);

    std::string beta_filter = Parameters::instance()->getStringParameter(BETA_RECORDING_PERSONS);
    std::shared_ptr<Filter<unsigned int>> beta_person_filter = create_filter<unsigned int>(beta_filter, clinics);
    std::string beta_fname =  output_directory + "/" + Parameters::instance()->getStringParameter(BETA_OUTPUT_FILE);
    BetaRecorder beta_recorder(beta_fname, rank_, beta_person_filter);

    std::shared_ptr<Filter<std::string>> choice_service_filter = create_filter<std::string>(Parameters::instance()->getStringParameter(CHOICE_SERVICE_CODES), clinics);
    ChoiceRecorder choice_recorder(choice_fname, rank_, choice_service_filter);

    std::string hrx_share_fname =  output_directory + "/" + Parameters::instance()->getStringParameter(HRX_SHARE_OUTPUT_FILE);
    HRXSharedRecorder hrx_shared_recorder(hrx_share_fname, rank_);

    std::string hrx_recv_fname =  output_directory + "/" + Parameters::instance()->getStringParameter(HRX_RECV_OUTPUT_FILE);
    HRXReceivedRecorder hrx_recv_recorder(hrx_recv_fname, rank_);

    std::string hrx_counts_fname =  output_directory + "/" + Parameters::instance()->getStringParameter(HRX_TAG_COUNTS_FILE);
    HRXTagCountRecorder hrx_tag_count_recorder(hrx_counts_fname, rank_);

    Statistics::initialize(choice_recorder, visit_recorder, dosing_recorder, beta_recorder, hrx_shared_recorder, 
        hrx_recv_recorder, hrx_tag_count_recorder, rank_);
    double beta_recording_frequency = Parameters::instance()->getDoubleParameter(BETA_RECORDING_FREQUENCY);

    if (beta_filter != "NONE") {
      runner.scheduleEvent(beta_recording_frequency + .1, beta_recording_frequency, Schedule::FunctorPtr(new MethodFunctor<CRXModel>(this, &CRXModel::logBetaScores)));
      // record betas at 0
      runner.scheduleEvent(0.1, Schedule::FunctorPtr(new MethodFunctor<CRXModel>(this, &CRXModel::logBetaScores)));
    }

    runner.scheduleEvent(1.1, 1, Schedule::FunctorPtr(new MethodFunctor<CRXModel>(this, &CRXModel::logHRXTagCounts)));
    runner.scheduleEndEvent(Schedule::FunctorPtr(new MethodFunctor<CRXModel>(this, &CRXModel::writeData)));
    runner.scheduleEvent(24.1, 24, Schedule::FunctorPtr(new MethodFunctor<CRXModel>(this, &CRXModel::writeData)));


    std::string info_net_output = "";
    if (Parameters::instance()->contains(INFO_NET_OUTPUT_FILE)) {
        std::string net_fname = Parameters::instance()->getStringParameter(INFO_NET_OUTPUT_FILE);
        info_net_output = output_directory + "/" + chi_sim::insert_in_file_name(net_fname, rank_);
    }

    NetworkStatsRecorder::initialize(info_net_output);
    runner.scheduleEndEvent(Schedule::FunctorPtr(new MethodFunctor<NetworkStatsRecorder>(NetworkStatsRecorder::instance(), &NetworkStatsRecorder::close)));

    std::string place_net_output = "";
    if (Parameters::instance()->contains(PLACE_NET_OUTPUT_FILE)) {
        std::string net_fname = Parameters::instance()->getStringParameter(PLACE_NET_OUTPUT_FILE);
        place_net_output = output_directory + "/" + chi_sim::insert_in_file_name(net_fname, rank_);
    }

    PlaceEdgeRecorder::initialize(place_net_output);
    runner.scheduleEndEvent(Schedule::FunctorPtr(new MethodFunctor<PlaceEdgeRecorder>(PlaceEdgeRecorder::instance(), &PlaceEdgeRecorder::close)));
}

CRXModel::CRXModel(repast::Properties& props, MPI_Datatype dt) :
        AbsModelT(props, dt), cal { }, acts_cache{}, propensity_map{}  {

    if (rank_ == 0) {
        std::cout << "Model Constructor Start Mem: " << get_mem() << std::endl;
    }

    ServiceCodeMap* code_map = ServiceCodeMap::instance();
    string places_codes_file = Parameters::instance()->getStringParameter(PLACES_SERVICE_CODES);
    string service_codes_file = Parameters::instance()->getStringParameter(SERVICE_CODES);
    create_service_code_map(service_codes_file, places_codes_file, *code_map);

    //FileOutput::initialize("debug.txt", rank_);

    string clinics_file = Parameters::instance()->getStringParameter(CLINICS_FILE);
    std::map<int, ClinicData> clinic_map;
    create_clinic_map(clinics_file, clinic_map);

    initializePopulation(*code_map, clinic_map);
    ScheduleRunner& runner = RepastProcess::instance()->getScheduleRunner();
    initializeStats(runner, *code_map, clinic_map);
    runner.scheduleEvent(1, 1,
            Schedule::FunctorPtr(new MethodFunctor<CRXModel>(this, &CRXModel::step)));
    // update the activity schedules every 24 hours before the next day starts
    runner.scheduleEvent(23.9, 24, Schedule::FunctorPtr(new MethodFunctor<CRXModel>(this, &CRXModel::resetActivitySchedules)));

    // monday
    cal.day_of_week = 1;

    if (rank_ == 0) {
        std::cout << "Model Constructor End Mem: " << get_mem() << std::endl;
    }
}

CRXModel::~CRXModel() {
    local_persons.clear();
	remote_persons.clear();
	places.clear();
	local_places.clear();
    ServiceCodeMap::instance()->reset();
    acts_cache.clear();
    Statistics::reset();
    //hrx_provider->clear();
    //delete hrx_provider.get();
    //hrx_provider = nullptr;
}

std::shared_ptr<DosingParameters> create_dosing_parameters() {

    float dosing_decay = (float)Parameters::instance()->getDoubleParameter(DOSING_DECAY);
    float dosing_doctor = (float)Parameters::instance()->getDoubleParameter(DOSING_DOCTOR);
    float dosing_psr = (float)Parameters::instance()->getDoubleParameter(DOSING_PSR);
    float dosing_nurse = (float)Parameters::instance()->getDoubleParameter(DOSING_NURSE);
    float dosing_resource = (float)Parameters::instance()->getDoubleParameter(DOSING_RESOURCE);
    float dosing_peer = (float)Parameters::instance()->getDoubleParameter(DOSING_PEER);
    float dosing_kappa = (float)Parameters::instance()->getDoubleParameter(DOSING_KAPPA);

    DosingParameters::Builder builder;
    return builder.decay(dosing_decay).doctor(dosing_doctor)
                .psr(dosing_psr).nurse(dosing_nurse).resource(dosing_resource).peer(dosing_peer).newPlace(dosing_kappa).build();
}

void CRXModel::writeData() {
    Statistics::instance()->writeBetaScores();
    Statistics::instance()->writeChoices();
    Statistics::instance()->writeVisits();
    Statistics::instance()->writeDosingEvents();
    Statistics::instance()->writeHRXSharedEvents();
    Statistics::instance()->writeHRXReceivedEvents();
    Statistics::instance()->writeHRXTagCounts();
}

void CRXModel::logBetaScores() {
    double tick = RepastProcess::instance()->getScheduleRunner().currentTick();
    Statistics* stats = Statistics::instance();
    for (auto& kv : local_persons) {
        PersonPtr p = kv.second;
        p->logBetas(tick, stats);
    }
    Statistics::instance()->writeBetaScores();
}

void CRXModel::logHRXTagCounts() {
    for (auto& kv : local_persons) {
        PersonPtr p = kv.second;
        p->logHRXTagCounts();
    }
    Statistics::instance()->writeHRXTagCounts();
}

void fill_propensity_values(PropensityValues& pv) {
    float multiplier = (float)Parameters::instance()->getDoubleParameter(PROPENSITY_MULTIPLIER); 
    pv.n_propensity = (float)Parameters::instance()->getDoubleParameter(PROPENSITY_NONE);
    pv.l_propensity = (float)Parameters::instance()->getDoubleParameter(PROPENSITY_LOW) * multiplier;
    pv.m_propensity = (float)Parameters::instance()->getDoubleParameter(PROPENSITY_MED) * multiplier;
    pv.h_propensity = (float)Parameters::instance()->getDoubleParameter(PROPENSITY_HIGH) * multiplier;

    //std::cout << "Propensity: " << pv.n_propensity << ", " << pv.l_propensity << ", " << pv.m_propensity << ", " 
    //    << pv.h_propensity << std::endl;

    pv.l_uncertainty = (float)Parameters::instance()->getDoubleParameter(UNCERTAINTY_LOW);
    pv.m_uncertainty = (float)Parameters::instance()->getDoubleParameter(UNCERTAINTY_MED);
    pv.h_uncertainty = (float)Parameters::instance()->getDoubleParameter(UNCERTAINTY_HIGH);
}

void CRXModel::initializePopulation(ServiceCodeMap& code_map, std::map<int, ClinicData>& clinic_map) {

    string service_deltas_file = Parameters::instance()->getStringParameter(SERVICE_DELTAS_FILE);
    float multiplier = (float)Parameters::instance()->getDoubleParameter(DELTA_MULTIPLIER); 
    double delta_low = Parameters::instance()->getDoubleParameter(DELTA_LOW) * multiplier;
    double delta_med = Parameters::instance()->getDoubleParameter(DELTA_MED) * multiplier;
    double delta_high = Parameters::instance()->getDoubleParameter(DELTA_HIGH) * multiplier;

    //std::cout << "Deltas: " << delta_low << ", " << delta_med << ", " << delta_high << std::endl;

    initialize_place_score_calculator(service_deltas_file, code_map, delta_low, delta_med, delta_high);

    std::map<int, PlacePtr> place_map;
    string places_file = Parameters::instance()->getStringParameter(PLACES_FILE);

  
    std::shared_ptr<HRXCache> hrx_provider = std::make_shared<HRXCache>();
    std::shared_ptr<DosingParameters> dosing_params = create_dosing_parameters();
    create_places(places_file, place_map, clinic_map, code_map, dosing_params, hrx_provider);

    string hrx_file = Parameters::instance()->getStringParameter(HRX_FILE);
    fill_hrx_cache(hrx_file, hrx_provider, place_map);

    string schedule_file = Parameters::instance()->getStringParameter(SCHEDULE_FILE);
    string acts_file = Parameters::instance()->getStringParameter(ACTIVITIES_FILE);
    string buckets_file = Parameters::instance()->getStringParameter(BUCKETS_FILE);

    std::map<std::string, float> gamma_map;

    gamma_map["LOW"] = (float)Parameters::instance()->getDoubleParameter(GAMMA_LOW);
    gamma_map["MED"] = (float)Parameters::instance()->getDoubleParameter(GAMMA_MED);
    gamma_map["HIGH"] =  (float)Parameters::instance()->getDoubleParameter(GAMMA_HIGH);

    create_activities(schedule_file, acts_file, buckets_file, code_map, acts_cache, gamma_map);

    // ActsPtr acts = acts_cache.getSchedule(10559);
    // for (int i = 0; i < 24; ++i) {
    //     std::shared_ptr<ScheduleEntry> act = acts->findActivityAt(i * 60);
    //     std::cout << i << ": " << act->start() << ", " << act->end() << ", " << act->alphaServiceCode() << ", " << act->act()->atus_code_ << std::endl;
    // }

    string persons_file = Parameters::instance()->getStringParameter(PERSONS_FILE);
    string alpha_file = Parameters::instance()->getStringParameter(PERSONS_ALPHA_FILE);
    CRXPersonCreatorHelper helper;
    create_persons(persons_file,  alpha_file, local_persons, place_map, acts_cache, code_map, helper);

    std::string init_places_file = Parameters::instance()->getStringParameter(INIT_PLACES_FILE);
    std::string init_clinics_file = Parameters::instance()->getStringParameter(INIT_CLINICS_FILE);
    assign_initial_places(init_places_file, init_clinics_file, local_persons, place_map);

    for (auto iter : place_map) {
        places.insert(make_pair(iter.second->placeId().place_id, iter.second));
        if (iter.second->placeId().rank == rank_) {
            local_places.push_back(iter.second);
        }
    }

    string props_file = Parameters::instance()->getStringParameter(PROPENSITY_FILE);
    PropensityValues pv;
    fill_propensity_values(pv);
    create_propensities(props_file, propensity_map, pv);

    //(*FileOutput::instance()) << "places count: " << local_places.size() << std::endl <<
    //        "person count: " << local_persons.size() << std::endl;
}


PersonPtr CRXModel::createPerson(unsigned int p_id, CRXPersonData& data) {
    Places p;
    PersonPtr person = std::make_shared<CRXPerson>(data.person_id, p, data.alpha, acts_cache, data.weekday_bucket_id,
            data.weekend_bucket_id, data.age_id);
    person->update(places, acts_cache, data);
    return person;
}

/**
 * Update the state of the specified person with the data in data.
 *
 */
void CRXModel::updatePerson(PersonPtr& person, CRXPersonData& data) {
    person->update(places, acts_cache, data);
}

void CRXModel::nextActSelected(PersonPtr& person, chi_sim::NextPlace<CRXPlace>& next_act) {}

void CRXModel::step() {

    cal.increment();
    double tick = RepastProcess::instance()->getScheduleRunner().currentTick();
    cal.tick = tick;
    Statistics::instance()->setTick(tick);
    NetworkStatsRecorder::instance()->initInfoNetworkRecording(tick);
    PlaceEdgeRecorder::instance()->initRecording(tick);
    //(*FileOutput::instance()) << tick << " move persons start" << std::endl;
    movePersons(cal);
    //(*FileOutput::instance()) << tick << " move persons end" << std::endl;
    //std::cout << "tick : " << tick << std::endl;

    int n = local_places.size();

    // schedule(dynamic)
    // #pragma omp parallel for
    for (int i = 0; i < n; ++i) {
        auto& place = local_places[i];
        place->run(propensity_map);
        //if (place->size() > 500) {
        //    std::cout << place->placeId().place_id << ": " << place->size() << std::endl;
        //}
        place->reset();
    }

    if (rank_ == 0 && (int) tick % 24 == 0) {
        std::string time;
        repast::timestamp(time);
        std::cout << "Day: " << cal.day_of_year << " " << time << " " << get_mem() << std::endl;
    }
}

void CRXModel::resetActivitySchedules() {
    // 5 is friday, and 6 is sat, so next day is sat. or sunday
    bool next_day_is_weekend = cal.day_of_week == 5 || cal.day_of_week == 6;
    if (next_day_is_weekend) {
        for (auto& kv : local_persons) {
            kv.second->resetWeekendActivitySchedule(acts_cache);
        }
    } else {
        for (auto& kv : local_persons) {
            kv.second->resetWeekdayActivitySchedule(acts_cache);
        }
    }
}

} /* namespace crx */
