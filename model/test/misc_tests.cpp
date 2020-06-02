#include <typeinfo>

#include "gtest/gtest.h"

#include "creators.h"
#include "CRXPlace.h"
#include "Places.h"
#include "ScheduleEntry.h"
#include "PlaceScoreCalculator.h"
#include "DosingParameters.h"

#include "mrsa/Activities.h"

using namespace crx;
using namespace chi_sim;

using EntryPtr = std::shared_ptr<ScheduleEntry>;

const float DECAY = 0.9;
const float DOCTOR = 0.3;
const float PSR = 0.4;
const float NURSE = 0.5;
const float RESOURCE = 0.2;
const float PEER = 0.6;
const float KAPPA = 0.001;

/*
struct CreatorHelper {

    PersonPtr createLocalPerson(unsigned int num_id, const Places& places, double alpha,
            ActivitiesCache& cache, unsigned int weekday_bucket, unsigned int weekend_bucket,
            std::vector<std::string>& row) {
        PersonPtr person = std::make_shared<CRXPerson>(num_id, places, alpha, cache, weekday_bucket, weekend_bucket);
        return person;
    }

    void processRemotePerson(unsigned int pid, int rank) {
    }
};
*/

TEST(CreatorTests, testClinicCreation) {
    std::map<int, PlacePtr> place_map;

    ServiceCodeMap code_map = *ServiceCodeMap::instance();;
    create_service_code_map("../test_data/service_codes.csv", "../test_data/test_places_to_service_codes.csv", code_map);

    DosingParameters::Builder builder;
    std::shared_ptr<DosingParameters> dosing_params = builder.decay(0.1).doctor(0.2).psr(0.3).nurse(
            0.4).resource(0.5).peer(0.6).newPlace(0.7).build();

    std::map<int, ClinicData> clinic_map;
    create_clinic_map("../test_data/clinics.csv", clinic_map);
    ASSERT_EQ((size_t )3, clinic_map.size());

    std::shared_ptr<HRXCache> hrx_provider = std::make_shared<HRXCache>();
    create_places("../test_data/test_places.csv", place_map, clinic_map, code_map, dosing_params, hrx_provider);
    PlacePtr place = place_map[371];
    ASSERT_EQ(CLINIC_TYPE, place->type());
}

TEST(PlaceSelectionTests, testCRXDosing) {
    DosingParameters::Builder builder;
    float dr_dosing = 0.2;
    float psr_dosing = 0.3;
    float nurse_dosing = 0.4;
    std::shared_ptr<DosingParameters> dosing_params = builder.decay(0.1).doctor(dr_dosing).psr(psr_dosing).
            nurse(nurse_dosing).resource(0.5).peer(0.6).newPlace(0.7).build();

    std::shared_ptr<HRXCache> hrx = std::make_shared<HRXCache>();

    ClinicHRX hrx1(hrx, 1.0, 0.0, 0.0);
    ASSERT_EQ(dr_dosing, hrx1.getDose(dosing_params));

    ClinicHRX hrx2(hrx, 0.0, 1.0, 0.0);
    ASSERT_EQ(nurse_dosing, hrx2.getDose(dosing_params));

    ClinicHRX hrx3(hrx, 0.0, 0.0, 1.0);
    ASSERT_EQ(psr_dosing, hrx3.getDose(dosing_params));
}

TEST(CreatorTests, testPlaceCreation) {
    std::map<int, PlacePtr> place_map;

    ServiceCodeMap code_map = *ServiceCodeMap::instance();;
    create_service_code_map("../test_data/service_codes.csv", "../test_data/test_places_to_service_codes.csv", code_map);

    DosingParameters::Builder builder;
    std::shared_ptr<DosingParameters> dosing_params = builder.decay(0.1).doctor(0.2).psr(0.3).
            nurse(0.4).resource(0.5).peer(0.6).newPlace(0.7).build();

    std::map<int, ClinicData> clinic_map;
    create_clinic_map("../test_data/clinics.csv", clinic_map);

    std::shared_ptr<HRXCache> hrx_provider = std::make_shared<HRXCache>();
    create_places("../test_data/test_places.csv", place_map, clinic_map, code_map, dosing_params, hrx_provider);
    ASSERT_EQ(23, place_map.size());

    auto place = place_map.at(789);
    ASSERT_EQ(0, place->placeId().rank);
    ASSERT_EQ(789, place->placeId().place_id);
    ASSERT_EQ(41.90805, place->location().latitude());
    ASSERT_EQ(-87.887165, place->location().longitude());
    ASSERT_EQ(60160, place->zipcode());
}

TEST(CreatorTests, testPlaces) {
    std::map<int, PlacePtr> place_map;
    ServiceCodeMap code_map = *ServiceCodeMap::instance();;
    create_service_code_map("../test_data/service_codes.csv", "../test_data/test_places_to_service_codes.csv", code_map);

    DosingParameters::Builder builder;
    std::shared_ptr<DosingParameters> dosing_params = builder.decay(0.1).doctor(0.2).psr(0.3).
            nurse(0.4).resource(0.5).peer(0.6).newPlace(0.7).build();

    std::map<int, ClinicData> clinic_map;
        create_clinic_map("../test_data/clinics.csv", clinic_map);

    std::shared_ptr<HRXCache> hrx_provider = std::make_shared<HRXCache>();
    create_places("../test_data/test_places.csv", place_map, clinic_map, code_map, dosing_params, hrx_provider);
    ASSERT_EQ(23, place_map.size());

    double beta = 0.5;
    std::vector<int> place_ids { 1, 171, 174, 242, 363, 464, 478, 789, 812 };

    Places p;
    for (int id : place_ids) {
        auto place = place_map.at(id);
        p.addPlace(place.get(), beta, false);
    }

    std::vector<double> data;
    p.fillSendData(data);

    std::vector<double> expected {  1, 0.5, 171, 0.5, 174, 0.5, 242, 0.5, 363, 0.5, 464,
        0.5, 478, 0.5, 789, 0.5, 812, 0.5};

    ASSERT_EQ(expected.size(), data.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        ASSERT_EQ(expected[i], data[i]);
    }

    reset_places(p, place_map, 0, data.size(), &data[0]);
    data.clear();
    p.fillSendData(data);
    ASSERT_EQ(expected.size(), data.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        ASSERT_EQ(expected[i], data[i]);
    }
}

TEST(CreatorTests, testActsCreation) {
    ServiceCodeMap code_map = *ServiceCodeMap::instance();;
    create_service_code_map("../test_data/service_codes.csv", "../test_data/test_places_to_service_codes.csv", code_map);
    std::map<std::string, float> gamma_map;
    gamma_map["LOW"] = 0.1;
    gamma_map["MED"] = 0.2;
    gamma_map["HIGH"] = 0.3;
    ActivitiesCache cache;
    create_activities("../test_data/test_activity_schedules.csv",
            "../test_data/test_activities.csv", "../test_data/test_activity_buckets.csv", code_map, cache, gamma_map);

    ASSERT_EQ(3, cache.actsSize());
    ASSERT_EQ(3, cache.bucketsSize());

    ActsPtr acts = cache.selectSchedule(2, 1);
    ASSERT_TRUE(acts->id() == 2);

    EntryPtr entry = acts->findActivityAt(520);
    //int code = SCHOOL_SERVICE_CODE
    //int code = acts->id() == 2 ? SCHOOL_SERVICE_CODE : HOME_SERVICE_CODE;
    ASSERT_EQ(SCHOOL_SERVICE_CODE, entry->serviceCode());

    acts = cache.selectSchedule(2, 2);
    ASSERT_TRUE(acts->id() == 2 || acts->id() == 3);
    entry = acts->findActivityAt(520);
    int code = acts->id() == 2 ? SCHOOL_SERVICE_CODE : HOME_SERVICE_CODE;
    ASSERT_EQ(code, entry->serviceCode());
}

void create_persons(std::map<unsigned int, PersonPtr>& person_map, std::map<int, PlacePtr>& place_map, std::shared_ptr<HRXCache> hrx_cache = std::make_shared<HRXCache>()) {
    ServiceCodeMap code_map = *ServiceCodeMap::instance();;
    create_service_code_map("../test_data/service_codes.csv", "../test_data/test_places_to_service_codes.csv", code_map);
    initialize_place_score_calculator("../test_data/test_service_deltas.csv", code_map, 0.5, 1,
                   1.5);

    DosingParameters::Builder builder;
    std::shared_ptr<DosingParameters> dosing_params = builder.decay(DECAY).doctor(DOCTOR).psr(PSR).
            nurse(NURSE).resource(RESOURCE).peer(PEER).newPlace(KAPPA).build();

    std::map<int, ClinicData> clinic_map;
    create_clinic_map("../test_data/clinics.csv", clinic_map);

    create_places("../test_data/test_places.csv", place_map, clinic_map, code_map, dosing_params, hrx_cache);

    std::map<std::string, float> gamma_map;
    gamma_map["LOW"] = 0.1;
    gamma_map["MED"] = 0.2;
    gamma_map["HIGH"] = 0.3;
    ActivitiesCache cache;
    create_activities("../test_data/test_activity_schedules.csv",
                "../test_data/test_activities.csv", "../test_data/test_buckets_person_creation.csv", code_map, cache, gamma_map);

    Network net;

    CRXPersonCreatorHelper helper(&net, 0);
    create_persons("../test_data/test_persons.csv", "../test_data/test_persons_alpha.csv", person_map, place_map, cache, code_map,
            helper);
}

TEST(CreatorTests, testPropensityValues) {
    PropensityValues pv;
    pv.n_propensity = 0.1f;
    pv.l_propensity = 0.2f;
    pv.m_propensity = 0.3f;
    pv.h_propensity = 0.4f;

    pv.l_uncertainty = 0.5f;
    pv.m_uncertainty = 0.6f;
    pv.h_uncertainty = 0.7f;

    ASSERT_EQ(0.1f, pv.getPropensity("N"));
    ASSERT_EQ(0.2f, pv.getPropensity("L"));
    ASSERT_EQ(0.3f, pv.getPropensity("M"));
    ASSERT_EQ(0.4f, pv.getPropensity("H"));

    ASSERT_EQ(0.5f, pv.getUncertainty("L"));
    ASSERT_EQ(0.6f, pv.getUncertainty("M"));
    ASSERT_EQ(0.7f, pv.getUncertainty("H"));

}

void create_propensity_map(std::map<unsigned int, NormalGenerator>& p_map) {
    PropensityValues pv;
    pv.n_propensity = 0;
    pv.l_propensity = 0.2f;
    pv.m_propensity = 0.3f;
    pv.h_propensity = 0.4f;

    pv.l_uncertainty = 0.0f;
    pv.m_uncertainty = 0.6f;
    pv.h_uncertainty = 0.7f;

    create_propensities("../test_data/propensity.csv", p_map, pv);

}

TEST(CreatorTests, testPropensityCreation) {
    // void create_propensities(const std::string& propensity_file, std::map<std::string, NormalGenerator>& propensity_map, PropensityValues& propensity_values)

    std::map<unsigned int, NormalGenerator> p_map;
    create_propensity_map(p_map);

    ASSERT_EQ(75, p_map.size());

    // H,M
    NormalGenerator g = p_map.at(30404);
    ASSERT_NEAR(0.4, g.distribution().mean(), 0.000001);
    ASSERT_NEAR(0.4 * 0.6, g.distribution().sigma(), 0.000001);

    // L,L
    NormalGenerator g2 = p_map.at(40301);
    ASSERT_NEAR(0.2, g2.distribution().mean(), 0.000001);
    ASSERT_NEAR(0, g2.distribution().sigma(), 0.000001);

    // N,L
    NormalGenerator g3 = p_map.at(10101);
    ASSERT_NEAR(0, g3.distribution().mean(), 0.000001);
    ASSERT_NEAR(0, g3.distribution().sigma(), 0.000001);
}

TEST(CreatorTests, testPersonCreation) {
    std::map<unsigned int, PersonPtr> person_map;
    std::map<int, PlacePtr> place_map;
    create_persons(person_map, place_map);
    ASSERT_EQ(9, person_map.size());

    auto p = person_map.at(2177573);
    ASSERT_EQ(2177573, p->id());

    std::vector<double> data;
    p->fillSendData(data);
    ASSERT_EQ(2177573.0, data[0]);
    // alpha value
    ASSERT_NEAR(0.479422757830965, data[1], 0.0000001);
    // weekday schedule
    ASSERT_EQ(1.0, data[2]);
    // weekend schedule
    ASSERT_EQ(3.0, data[3]);
    // weekday bucket
    ASSERT_EQ(1.0, data[4]);
    // weekend bucket
    ASSERT_EQ(3.0, data[5]);
}

TEST(CreatorTests, testInitialPlaceAssignment) {
    std::map<unsigned int, PersonPtr> person_map;
    std::map<int, PlacePtr> place_map;
    create_persons(person_map, place_map);

    assign_initial_places("../test_data/init_places.csv", "../test_data/init_clinics.csv",
            person_map, place_map);

    PersonPtr p1 = person_map[2177570];
    PersonPtr p2 = person_map[2177571];
    PersonPtr p3 = person_map[2177572];

    ASSERT_EQ(0.028, p1->getBeta(364));
    ASSERT_EQ(0.982, p1->getBeta(365));
    ASSERT_EQ(0.2, p1->getBeta(370));
    ASSERT_EQ("clinic", place_map[370]->type());

    ASSERT_EQ(0.13, p2->getBeta(366));
    ASSERT_EQ(0.34, p2->getBeta(369));
    ASSERT_EQ(0.1, p2->getBeta(371));
    ASSERT_EQ("clinic", place_map[371]->type());

    ASSERT_EQ(0.92, p3->getBeta(368));
    ASSERT_EQ(0.3, p3->getBeta(372));
    ASSERT_EQ("clinic", place_map[372]->type());
}

void init_stats() {
    std::string beta_fname("../test_data/beta_out.txt");
    std::string choice_fname("../test_data/choice_out.txt");
    ResourceVisitRecorder recorder("/dev/null", 0);
    Statistics::initialize("/dev/null", "/dev/null", recorder, 0);
}

TEST(PlaceSelectionTests, mapTests) {
    ServiceCodeMap code_map = *ServiceCodeMap::instance();;
    create_service_code_map("../test_data/service_codes.csv", "../test_data/test_places_to_service_codes.csv", code_map);
    initialize_place_score_calculator("../test_data/test_service_deltas.csv", code_map, 0.5, 1, 1.5);

    ASSERT_NE(nullptr, PlaceScoreCalculator::instance());

    // W,100,200
    // N05,1,5
    // A15,2.5,7.5
    ASSERT_EQ(Delta::LOW, PlaceScoreCalculator::instance()->getDelta(code_map.getNumericServiceCode("W"), 0));
    ASSERT_EQ(Delta::MEDIUM, PlaceScoreCalculator::instance()->getDelta(code_map.getNumericServiceCode("W"), 100.1));
    ASSERT_EQ(Delta::HIGH, PlaceScoreCalculator::instance()->getDelta(code_map.getNumericServiceCode("W"), 200.1));

    ASSERT_EQ(Delta::LOW, PlaceScoreCalculator::instance()->getDelta(code_map.getNumericServiceCode("N05"), .99));
    ASSERT_EQ(Delta::MEDIUM, PlaceScoreCalculator::instance()->getDelta(code_map.getNumericServiceCode("N05"), 5));
    ASSERT_EQ(Delta::HIGH, PlaceScoreCalculator::instance()->getDelta(code_map.getNumericServiceCode("N05"), 5.1));

    ASSERT_EQ(Delta::LOW, PlaceScoreCalculator::instance()->getDelta(code_map.getNumericServiceCode("A15"), 2.5));
    ASSERT_EQ(Delta::MEDIUM, PlaceScoreCalculator::instance()->getDelta(code_map.getNumericServiceCode("A15"), 2.6));
    ASSERT_EQ(Delta::HIGH, PlaceScoreCalculator::instance()->getDelta(code_map.getNumericServiceCode("A15"), 7.6));
}

// Tests PlaceScoreCalc scoring
TEST(PlaceSelectionTests, scoreTests) {
    ServiceCodeMap code_map = *ServiceCodeMap::instance();;
    create_service_code_map("../test_data/service_codes.csv", "../test_data/test_places_to_service_codes.csv", code_map);
    initialize_place_score_calculator("../test_data/test_service_deltas.csv", code_map, 0.5, 1, 1.5);

    int work_service_code = code_map.getNumericServiceCode("W");
    GeoPoint p1(41.77996, -87.70298);
    GeoPoint p2(42.02039, -87.66674);
    ASSERT_NEAR(16.716342912481899, p1.distance(p2), 0);

    double beta = 0.2;
    double gamma = 0.1;
    double score = PlaceScoreCalculator::instance()->calculateScore(beta, gamma, work_service_code, p1, p2);

    // distance delta is low which should equal 0.5
    double delta_val = 0.5;
    double expected = beta / (gamma * delta_val);
    ASSERT_EQ(expected, score);

    // make the distance larger so use high delta val
    score = PlaceScoreCalculator::instance()->calculateScore(beta, gamma, work_service_code, p1, GeoPoint(42.02039, -72.66674));

    delta_val = 1.5;
    expected = beta / (gamma * delta_val);
    ASSERT_EQ(expected, score);
}

int find_best_score(std::map<int, double>& scores) {
    int id = 0;
    double max_score = -1;
    for (auto entry : scores) {
        if (entry.second > max_score) {
            max_score = entry.second;
            id = entry.first;
        } else if (entry.second == max_score) {
            std::cout << "tie" << std::endl;
        }
    }

    return id;
}



// Tests selecting place in Places
TEST(PlaceSelectionTests, placeSelectionTests) {
    init_stats();
    std::map<int, PlacePtr> place_map;
    //ServiceCodeMap code_map = *ServiceCodeMap::instance();;
    create_service_code_map("../test_data/service_codes.csv", "../test_data/test_places_to_service_codes.csv", *ServiceCodeMap::instance());
    initialize_place_score_calculator("../test_data/test_service_deltas.csv", *ServiceCodeMap::instance(), 0.5, 1,
            1.5);

    DosingParameters::Builder builder;
    std::shared_ptr<DosingParameters> dosing_params = builder.decay(0.1).doctor(0.2).psr(0.3).
            nurse(0.4).resource(0.5).peer(0.6).newPlace(0.7).build();

    std::map<int, ClinicData> clinic_map;
    create_clinic_map("../test_data/clinics.csv", clinic_map);

    std::shared_ptr<HRXCache> hrx_provider = std::make_shared<HRXCache>();
    create_places("../test_data/test_places.csv", place_map, clinic_map, *ServiceCodeMap::instance(), dosing_params, hrx_provider);

    std::vector<int> place_ids { 364, 171, 174, 242, 363, 464, 365, 366, 367 };
    std::vector<double> betas { 0.1, 0.12, 0.13, 0.14, 0.2, 0.21, 0.05, 0.01, 0.003 };
    int w_code = ServiceCodeMap::instance()->getNumericServiceCode("A15");
    // atus, dtype, gamma
    std::shared_ptr<Activity> act = std::make_shared<Activity>(101, DecisionType::A, 0.2);

    Places p;
    int i = 0;
    std::shared_ptr<CRXPlace> home = place_map.at(174);
    GeoPoint home_loc = home->location();
    std::map<int, double> scores;
    for (int id : place_ids) {
        auto place = place_map.at(id);
        p.addPlace(place, betas[i], true);
        Delta delta = PlaceScoreCalculator::instance()->getDelta(w_code,
                home_loc.distance(place->location()));
        double delta_val = delta == Delta::LOW ? 0.5 : delta == Delta::MEDIUM ? 1 : 1.5;
        double score = betas[i] / (act->gamma_ * delta_val);
        if (place->type() == "service") {
            scores.emplace(id, score);
        }
        ++i;
    }

    ActInfo act_info{home.get(), 0};

    // get best score for A15
    p.selectPlace(w_code, 0, act_info, act);
    int best_id = find_best_score(scores);
    ASSERT_EQ(place_map[best_id]->type(), "service");
    ASSERT_EQ(place_map[best_id].get(), act_info.place);

    // alpha > score but decision type is A so shouldn't matter
    p.selectPlace(w_code, 10, act_info, act);
    best_id = find_best_score(scores);
    ASSERT_EQ(place_map[best_id].get(), act_info.place);

    // alpha > score, decision type AB, so stay with current place (i.e. home)
    act_info.place = home.get();
    act->decision_type = DecisionType::AB;
    p.selectPlace(w_code, 10, act_info, act);
    ASSERT_EQ(home.get(), act_info.place);

    // alpha < score, decision type AB, so highest workplace
    act_info.place = home.get();
    p.selectPlace(w_code, 1.99, act_info, act);
    best_id = find_best_score(scores);
    ASSERT_EQ(place_map[best_id].get(), act_info.place);
}

// Tests person selectNextPlace
TEST(PlaceSelectionTests, nextPlaceTests) {
   init_stats();
   std::map<unsigned int, PersonPtr> person_map;
   std::map<int, PlacePtr> place_map;
   create_persons(person_map, place_map);

   ASSERT_EQ(9, person_map.size());

   auto p = person_map.at(2177573);
   ASSERT_EQ(2177573, p->id());

    Calendar cal;
    NextPlace<CRXPlace> next_act;
    // cal starts on Sunday, stays home all weekend
    for (int i = 0; i < 1440; ++i) {
        cal.minute_of_day = i;
        p->selectNextPlace(cal, next_act);
        ASSERT_EQ(1011187, next_act.id.place_id);
        ASSERT_TRUE(next_act.place != nullptr);
        ASSERT_EQ(1011187, next_act.place->placeId().place_id);
    }

    // weekday at work 8:30 AM (510) - 5pm (1020)
    cal.day_of_week = 1;
    for (int i = 0; i < 510; ++i) {
        cal.minute_of_day = i;
        p->selectNextPlace(cal, next_act);
        ASSERT_EQ(1011187, next_act.id.place_id);
        ASSERT_TRUE(next_act.place != nullptr);
        ASSERT_EQ(1011187, next_act.place->placeId().place_id);
    }

    // at work
    for (int i = 510; i < 1020; ++i) {
        cal.minute_of_day = i;
        p->selectNextPlace(cal, next_act);
        ASSERT_EQ(478, next_act.id.place_id);
        ASSERT_TRUE(next_act.place != nullptr);
        ASSERT_EQ(478, next_act.place->placeId().place_id);
    }

    // back home
    for (int i = 1020; i < 1440; ++i) {
        cal.minute_of_day = i;
        p->selectNextPlace(cal, next_act);
        ASSERT_EQ(1011187, next_act.id.place_id);
        ASSERT_TRUE(next_act.place != nullptr);
        ASSERT_EQ(1011187, next_act.place->placeId().place_id);
    }

    p = person_map.at(2177578);
    ASSERT_EQ(2177578, p->id());
    for (int i = 0; i < 510; ++i) {
        cal.minute_of_day = i;
        p->selectNextPlace(cal, next_act);
        ASSERT_EQ(1011189, next_act.id.place_id);
        ASSERT_TRUE(next_act.place != nullptr);
        ASSERT_EQ(1011189, next_act.place->placeId().place_id);
    }

    // at work
    for (int i = 510; i < 900; ++i) {
        cal.minute_of_day = i;
        p->selectNextPlace(cal, next_act);
        ASSERT_EQ(242, next_act.id.place_id);
        ASSERT_TRUE(next_act.place != nullptr);
        ASSERT_EQ(242, next_act.place->placeId().place_id);
    }

    // back home
    for (int i = 900; i < 1440; ++i) {
        cal.minute_of_day = i;
        p->selectNextPlace(cal, next_act);
        ASSERT_EQ(1011189, next_act.id.place_id);
        ASSERT_TRUE(next_act.place != nullptr);
        ASSERT_EQ(1011189, next_act.place->placeId().place_id);
    }

    // saturday, weekend schedule, at home
    cal.day_of_week = 6;
    for (int i = 0; i < 1440; ++i) {
        cal.minute_of_day = i;
        p->selectNextPlace(cal, next_act);
        ASSERT_EQ(1011189, next_act.id.place_id);
        ASSERT_TRUE(next_act.place != nullptr);
        ASSERT_EQ(1011189, next_act.place->placeId().place_id);
    }
}

TEST(PlaceSelectionTests, testPlaceToPlaceDosing) {
    init_stats();
    std::map<int, PlacePtr> place_map;
    ServiceCodeMap code_map = *ServiceCodeMap::instance();;
    create_service_code_map("../test_data/service_codes.csv", "../test_data/test_places_to_service_codes.csv", code_map);

    DosingParameters::Builder builder;
    std::shared_ptr<DosingParameters> dosing_params = builder.decay(0.1).doctor(0.2).psr(0.3).
            nurse(0.4).resource(0.5).peer(0.6).newPlace(0.7).build();

    std::map<int, ClinicData> clinic_map;
    create_clinic_map("../test_data/clinics.csv", clinic_map);
    std::shared_ptr<HRXCache> hrx_provider = std::make_shared<HRXCache>();
    create_places("../test_data/test_places.csv", place_map, clinic_map, code_map, dosing_params, hrx_provider);

    Places p1;
    double init_beta = 0.027;
    p1.addPlace(place_map[1], init_beta, true);
    p1.addPlace(place_map[171], init_beta, true);

    Places p2;
    p2.addPlace(place_map[363], init_beta, true);
    p2.addPlace(place_map[171], init_beta, true);
    // add a household that shouldn't be shared
    p2.addPlace(place_map[1011185], init_beta, true);

    double dose = 0.6;
    p1.dose(p2, dose, init_beta);
    ASSERT_NEAR(pow(init_beta, dose), p1.getBeta(171), 0.000001);
    ASSERT_EQ(3, p1.size());

    p1.decayBetas(0.4);

    ASSERT_EQ(3, p1.size());
    ASSERT_NEAR(pow(init_beta, dose), p1.getBeta(363), 0.000001);
}

TEST(PlaceSelectionTests, testDosing) {
    init_stats();
    std::map<int, PlacePtr> place_map;
    ServiceCodeMap code_map = *ServiceCodeMap::instance();;
    create_service_code_map("../test_data/service_codes.csv", "../test_data/test_places_to_service_codes.csv", code_map);

    DosingParameters::Builder builder;
    std::shared_ptr<DosingParameters> dosing_params = builder.decay(0.1).doctor(0.2).psr(0.3).
            nurse(0.4).resource(0.5).peer(0.6).newPlace(0.7).build();

    std::map<int, ClinicData> clinic_map;
    create_clinic_map("../test_data/clinics.csv", clinic_map);
    std::shared_ptr<HRXCache> hrx_provider = std::make_shared<HRXCache>();
    create_places("../test_data/test_places.csv", place_map, clinic_map, code_map, dosing_params, hrx_provider);

    double init_beta = 0.027;
    std::vector<int> place_ids { 1, 171, 174, 242, 363, 464, 478, 789 };

    Places p;
    for (int id : place_ids) {
        auto place = place_map.at(id);
        p.addPlace(place.get(), init_beta, true);
    }

    for  (int id : place_ids) {
        ASSERT_EQ(init_beta, p.getBeta(id));
    }

    PlacePtr place = place_map[242];
    p.dose(place.get(), 0.3, 0);
    double new_beta = pow(init_beta, 0.3);
    ASSERT_NEAR(new_beta, p.getBeta(242), 0.0000001);
    p.decayBetas(0.9);

    for  (int id : place_ids) {
        double exp = id == 242 ? new_beta * 0.9 : init_beta * 0.9;
        ASSERT_NEAR(exp, p.getBeta(id), 0.0000001);
    }

    double kappa = 0.04;
    // note that new one won't be added until we decay
    p.dose(place_map.at(1011189).get(), 0.6, kappa);
    ASSERT_EQ(place_ids.size() + 1, p.size());
    p.decayBetas(0.9);
    ASSERT_EQ(place_ids.size() + 1, p.size());
    // no decay when first added
    double exp = pow(kappa, 0.6);
    ASSERT_NEAR(exp, p.getBeta(1011189), 0.0000001);
    p.decayBetas(0.9);
    ASSERT_NEAR(exp * 0.9, p.getBeta(1011189), 0.000001);
}

TEST(PlaceSelectionTests, testDosingParameters) {
    DosingParameters::Builder builder;
    std::shared_ptr<DosingParameters> p = builder.build();
    ASSERT_TRUE(p);
    ASSERT_EQ(0.0f, p->decay());
    ASSERT_EQ(0.0f, p->doctor());
    ASSERT_EQ(0.0f, p->psr());
    ASSERT_EQ(0.0f, p->nurse());
    ASSERT_EQ(0.0f, p->resource());
    ASSERT_EQ(0.0f, p->peer());
    ASSERT_EQ(0.0f, p->newPlace());

    std::shared_ptr<DosingParameters> p1 = builder.decay(0.1).doctor(0.2).psr(0.3).
            nurse(0.4).resource(0.5).peer(0.6).newPlace(0.7).build();
    // make sure p is ok after building a new one
    ASSERT_TRUE(p);
    ASSERT_EQ(0.0f, p->decay());
    ASSERT_EQ(0.0f, p->doctor());
    ASSERT_EQ(0.0f, p->psr());
    ASSERT_EQ(0.0f, p->nurse());
    ASSERT_EQ(0.0f, p->resource());
    ASSERT_EQ(0.0f, p->peer());
    ASSERT_EQ(0.0f, p->newPlace());

    ASSERT_TRUE(p1);
    ASSERT_EQ(0.1f, p1->decay());
    ASSERT_EQ(0.2f, p1->doctor());
    ASSERT_EQ(0.3f, p1->psr());
    ASSERT_EQ(0.4f, p1->nurse());
    ASSERT_EQ(0.5f, p1->resource());
    ASSERT_EQ(0.6f, p1->peer());
    ASSERT_EQ(0.7f, p1->newPlace());
}

TEST(PlaceSelectionTests, testInPlaceDosing) {
   std::map<unsigned int, PersonPtr> person_map;
   std::map<int, PlacePtr> place_map;
   create_persons(person_map, place_map);

   // 2177576,812
   // 2177577,171
   // 2177578,363

   Calendar cal;
   NextPlace<CRXPlace> next_act;
   PersonPtr p1 = person_map.at(2177576);
   // need to do this so person has a current_act from which to get the
   // atus code required for getting info sharing propensity
   p1->selectNextPlace(cal, next_act);
   PersonPtr p2 = person_map.at(2177577);
   p2->selectNextPlace(cal, next_act);
   PersonPtr p3 = person_map.at(2177578);
   p3->selectNextPlace(cal, next_act);

   // same value in creators.cpp
   double beta = 0.027;

   // put persons in 812 --
   // p1 should dose resource from init beta
   // p2 should dose resource from new place kappa
   // p3 should dose resource from new place kappa
   PlacePtr place = place_map.at(812);
   place->addPerson(p1, 0);
   place->addPerson(p2, 0);
   place->addPerson(p3, 0);

   ASSERT_NEAR(pow(beta, RESOURCE), p1->getBeta(812), 0.000001);

   std::map<unsigned int, NormalGenerator> p_map;
   create_propensity_map(p_map);

   place->run(p_map);
   ASSERT_NEAR(pow(beta, RESOURCE) * DECAY, p1->getBeta(812), 0.000001);
   ASSERT_NEAR(pow(KAPPA, RESOURCE), p2->getBeta(812), 0.000001);
   ASSERT_NEAR(pow(KAPPA, RESOURCE), p3->getBeta(812), 0.000001);

   place->reset();
}

TEST(PlaceSelectionTests, testClinicDosing) {
    std::map<unsigned int, PersonPtr> person_map;
    std::map<int, PlacePtr> place_map;
    std::shared_ptr<HRXCache> hrx_cache = std::make_shared<HRXCache>();

    create_persons(person_map, place_map, hrx_cache);
    std::map<unsigned int, NormalGenerator> p_map;
    create_propensity_map(p_map);

    PersonPtr p1 = person_map.at(2177576);
    // add some places to list of places person knows about
    std::vector<int> place_ids {789, 363, 464, 478};
    for (int id : place_ids) {
        // dose of 1 means beta should equal KAPPA.
        p1->dose(place_map[id].get(), 1, KAPPA);
        // add them to the cache as well so hrx will return them
    }
    // decay necessary to add the new ones
    p1->decay(DECAY);
    for (int id : place_ids) {
        ASSERT_EQ(KAPPA, p1->getBeta(id));
    }

    // add places for p1 to hrx cache
    // add one person doesn't yet know about
    hrx_cache->putPlace(p1->id(), place_map[1]);
    // add ones person knows about aleady
    for (int id : place_ids) {
        hrx_cache->putPlace(p1->id(), place_map[id]);
    }

    Calendar cal;
    NextPlace<CRXPlace> next_act;
    // need to do this so person has a current_act from which to get the
    // atus code required for getting info sharing propensity
    p1->selectNextPlace(cal, next_act);

    // put person in clinic
    PlacePtr place = place_map[371];
    place->addPerson(p1, 0);

    // distributes hrx, info sharing and then decay
    place->run(p_map);

    // clinics
    // numeric_id   case_id hrx_probability doctor_probability  nurse_probability   psr_probability
    // 370 113085  1   1   0   0
    // 371 119007  1   0   1   0
    // 372 119460  0   0.3 0.3 0.4
    for (int id : place_ids) {
        // known places dosed by nurse
        ASSERT_EQ(DECAY * pow(KAPPA, NURSE), p1->getBeta(id));
    }
    // place 1 doesn't decay because just adde via hrx
    ASSERT_EQ(pow(KAPPA, NURSE), p1->getBeta(1));


    // try agin with clinic with crx prob of 0, should be no dosing
    PersonPtr p2 = person_map.at(2177577);
    for (int id : place_ids) {
        // dose of 1 means beta should equal KAPPA.
        p2->dose(place_map[id].get(), 0.4, KAPPA);
        // add them to the cache as well so hrx will return them
    }
    // decay necessary to add the new ones
    p2->decay(DECAY);
    for (int id : place_ids) {
        ASSERT_NEAR(pow(KAPPA, 0.4), p2->getBeta(id), 0.00000001);
    }

    // add places for p2 to hrx cache
    // add one person doesn't yet know about
    hrx_cache->putPlace(p2->id(), place_map[1]);
    // add ones person knows about aleady
    for (int id : place_ids) {
        hrx_cache->putPlace(p2->id(), place_map[id]);
    }

    // need to do this so person has a current_act from which to get the
    // atus code required for getting info sharing propensity
    p2->selectNextPlace(cal, next_act);

    // clinic with crx prob of 0
    place = place_map[372];
    // put person in clinic
    place->addPerson(p2, 0);

    // distributes hrx, info sharing and then decay
    place->run(p_map);
    // crx prob is 0, so no boost, only t-1 val decayed.
    for (int id : place_ids) {
        // known places dosed by nurse
        ASSERT_NEAR(DECAY * pow(KAPPA, 0.4), p2->getBeta(id), 0.00001);
    }
}



TEST(PlaceSelectionTests, testPlaceCacheRemoval) {
    std::map<int, PlacePtr> place_map;
    ServiceCodeMap code_map = *ServiceCodeMap::instance();
    create_service_code_map("../test_data/service_codes.csv", "../test_data/test_places_to_service_codes.csv", code_map);

    DosingParameters::Builder builder;
    std::shared_ptr<DosingParameters> dosing_params = builder.decay(0.1).doctor(0.2).psr(0.3).nurse(
            0.4).resource(0.5).peer(0.6).newPlace(0.7).build();

    std::map<int, ClinicData> clinic_map;
    create_clinic_map("../test_data/clinics.csv", clinic_map);
    std::shared_ptr<HRXCache> hrx_provider = std::make_shared<HRXCache>();
    create_places("../test_data/test_places.csv", place_map, clinic_map, code_map, dosing_params, hrx_provider);

    std::vector<int> place_ids { 1, 174, 364, 365, 366, 242, 367, 368};

    Places p;
    int b = 0;
    for (int id : place_ids) {
        auto place = place_map.at(id);
        p.addPlace(place.get(), b++, true);
    }

    ASSERT_EQ(8, p.size());

    float init_beta = 0.27;
    for (int i = 369; i < 373; ++i) {
        p.dose(place_map.at(i).get(), 0.1, init_beta);
    }

    ASSERT_EQ(12, p.size());

    // decay will add new ones
    p.decayBetas(1);
    ASSERT_EQ(MAX_PLACES, p.size());

    std::vector<int> removed { 364, 365, 369 };
    for (int id : removed) {
        ASSERT_EQ(NA_BETA, p.getBeta(id));
    }

    std::vector<int> not_removed { 1, 174, 242, 367, 368, 366, 370, 371, 372 };
    for (int id : not_removed) {
        ASSERT_TRUE(p.getBeta(id) != NA_BETA);
    }
}



