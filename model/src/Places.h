/*
 * Places.h
 *
 *  Created on: Oct 31, 2017
 *      Author: nick
 */

#ifndef PLACES_H_
#define PLACES_H_

#include <map>
#include <vector>

#include "boost/container/flat_set.hpp"
#include "chi_sim/PlaceId.h"

#include "CRXPlace.h"
#include "ScheduleEntry.h"
#include "Statistics.h"
#include "crx_person_data.h"
#include "typedefs.h"
#include "place_source.h"

const int NA_BETA = -99999;

namespace crx {

struct ActInfo {

    CRXPlace* place;
    unsigned int atus_code;
    int service_code;
};


class Places;

/*
struct ToAdd {
    CRXPlace* place_;
    float dose;
    double beta_;
    int count;

};
*/

class PlaceData {

private:
    friend Places;
    CRXPlace* place_;
    double beta_;
    bool do_decay;
    Source source_;

public:

    PlaceData(CRXPlace* place, double beta, bool decay, Source source = Source::OTHER);
    void doseBeta(float dose, Source source);
    void decayBeta(float decay);

    CRXPlace* place() {
        return place_;
    }

    double beta() {
        return beta_;
    }

    Source source() const {
        return source_;
    }
};

class Places {

private:
    // k: place id, v: place data for that place
    std::map<int, std::shared_ptr<PlaceData>> places_map;

    CRXPlace* home_, * school_, * work_, *initial_clinic;
    //std::map<int, ToAdd> to_add;
    int non_perm_count;

    std::shared_ptr<PlaceData> doAddPlace(CRXPlace* place, double beta, bool decay);
    // remove the count number of lowest beta places
    void remove(int count);
    void doSelectPlace(int service_code, const std::string& alpha_service_code, double alpha, ActInfo& current_act, std::shared_ptr<Activity> act, std::vector<std::shared_ptr<PlaceData>>& vec);

    void doSchool(int service_code, ActInfo& current_act, std::shared_ptr<Activity> act);
    void doHome(int service_code, ActInfo& current_act, std::shared_ptr<Activity> act);
    void doEverywhere(int service_code, ActInfo& current_act, std::shared_ptr<Activity> act);
    void doWork(int service_code, ActInfo& current_act, std::shared_ptr<Activity> act);

public:
    Places();
    void addPlace(CRXPlace* place, double beta, bool decay);
    void addPlace(std::shared_ptr<CRXPlace>& place, double beta, bool decay);
    void setInitialClinic(std::shared_ptr<CRXPlace>& place);

    size_t size() {
        return places_map.size();
    }

    void fillSendData(CRXPersonData& data);

    //void getPlaces(std::unordered_set<CRXPlace*>& place_set);
    void doseShared(boost::container::flat_set<int>& added_places,  Places& other, float dose, float initial_beta);

    void clear() {
        //code_places_map.clear();
        places_map.clear();
        non_perm_count = 0;
    }

    CRXPlace* home() {
        return home_;
    }

    void selectPlace(std::shared_ptr<ActT> entry, double alpha, ActInfo& current_act);

    /**
     * Doses the specified place. If the place is not yet known, then it is added with specified beta.
     */
    void dose(CRXPlace* place, float dose, float initial_beta, Source source);
    void decayBetas(float decay);

    /**
     * Gets the current beta value for the specified place.
     */
    double getBeta(int place_id);

    void logBetas(double tick, unsigned int person_id, Statistics* stats);
    void logHRXTagCounts(unsigned int person_id);

    /**
     * Resets the places using the info the CRXPersonData object.
     */
    void reset(std::map<int, std::shared_ptr<CRXPlace>>& place_map, CRXPersonData& data, size_t array_size);
};



} /* namespace crx */

#endif /* PLACES_H_ */
