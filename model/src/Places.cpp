/*
 * Places.cpp
 *
 *  Created on: Oct 31, 2017
 *      Author: nick
 */

#include <cmath>

#include "repast_hpc/Random.h"

#include "Places.h"
#include "constants.h"
#include "PlaceScoreCalculator.h"
#include "typedefs.h"

namespace crx {

const int NULL_PLACE_ID = -1;

PlaceData::PlaceData(CRXPlace* place, double beta, bool decay, Source source) :
        place_{place}, beta_{beta}, do_decay{decay}, source_{source} {
}

void PlaceData::doseBeta(float dose, Source source) {
    //std::cout << "place: " << place_->placeId().place_id <<  ", b: " << beta_ << ", dose: " << dose << std::endl;
    beta_ = pow(beta_, dose * (1.0 - beta_) + beta_);
    if (source_ == Source::OTHER) {
        source_ = source;
    } else if ((source_ == Source::HRX && source == Source::PEER) ||  
                (source_ == Source::PEER && source == Source::HRX)) {
        source_ = Source::HRX_PEER;
    } 
    //std::cout << "place: " << place_->placeId().place_id <<  ", b: " << beta_ << ", dose: " << dose << std::endl;
}

void PlaceData::decayBeta(float decay) {
    if (do_decay) {
        double tmp_beta = beta_;
        beta_ *= decay;
        // stats
        Statistics::instance()->recordDosingEvent(place_->placeId().place_id, decay, tmp_beta, beta_);
    }
    do_decay = true;
}

Places::Places() : places_map{}, home_{nullptr}, school_{nullptr}, work_{nullptr}, initial_clinic{nullptr}, non_perm_count{0}  {

}

void Places::logBetas(double tick, unsigned int person_id, Statistics* stats) {
    for (auto& kv : places_map) {
        stats->recordBetaScore(tick, person_id, kv.first, kv.second->beta());
    }
}

void Places::logHRXTagCounts(unsigned int person_id) {
    unsigned int hrx_count = 0, peer_count = 0, hrx_peer_count = 0;
    for (auto& kv : places_map) {
        const Source source = kv.second->source();
        if (source != Source::OTHER) {
            if (source == Source::HRX) {
                ++hrx_count;
            } else if (source == Source::PEER) {
                ++peer_count;
            } else if (source == Source::HRX_PEER) {
                ++hrx_peer_count;
            }

        }
    }
    
    Statistics* stats = Statistics::instance();
    stats->recordHRXTagCount(person_id, Source::HRX, hrx_count);
    stats->recordHRXTagCount(person_id, Source::PEER, peer_count);
    stats->recordHRXTagCount(person_id, Source::HRX_PEER, hrx_peer_count);
}

void Places::doseShared(boost::container::flat_set<int>& added_places, Places& other, float dose, float initial_beta) {
    bool log = true;
    for (auto& kv : places_map) {
        CRXPlace* place = kv.second->place();
        if (added_places.find(place->placeId().place_id) == added_places.end()) {
            //places_to_add.push_back(kv.second->place());
            added_places.emplace(place->placeId().place_id);
            if (!place->isHomeWorkSchool()) {
                Source source = kv.second->source();
                if (source != Source::OTHER) {
                    // From an HRX at some point down the line, now pass as a peer
                    source = Source::PEER;
                    if (log) {
                        Statistics::instance()->recordHRXSharedEvent();
                        log = false;
                    }
                }
                other.dose(place, dose, initial_beta, source);
            }
        }
    }

    // std::transform(places_map.begin(), places_map.end(),
    //         std::inserter(place_set, place_set.begin()),
    //         [](const std::pair<int, std::shared_ptr<PlaceData>>& key_value) {
    //             return key_value.second->place();
    //         });
}

// assumes that place is not already in Places
std::shared_ptr<PlaceData> Places::doAddPlace(CRXPlace* place, double beta, bool decay) {

    if (!(place->isHomeWorkSchool() || place == initial_clinic)) {
        ++non_perm_count;
    }
    std::shared_ptr<PlaceData> pd = std::make_shared<PlaceData>(place, beta, decay);
    places_map.emplace(place->placeId().place_id, pd);
    return pd;
}

void Places::addPlace(CRXPlace* place, double beta, bool decay) {
    auto iter = places_map.find(place->placeId().place_id);
    if (iter == places_map.end()) {
        doAddPlace(place, beta, decay);
    }
}

void Places::addPlace(std::shared_ptr<CRXPlace>& place, double beta, bool decay) {
    if (place->type() == HOUSEHOLD_TYPE) {
        home_ = place.get();
    } else if (place->type() == SCHOOL_TYPE) {
        school_ = place.get();
    } else if (place->type() == WORKPLACE_TYPE) {
        work_ = place.get();
    }
    auto iter = places_map.find(place->placeId().place_id);
    if (iter == places_map.end()) {
        doAddPlace(place.get(), beta, decay);
    }
}

void Places::setInitialClinic(std::shared_ptr<CRXPlace>& place) {
    initial_clinic = place.get();
}


void Places::dose(CRXPlace* place, float dose, float initial_beta, Source source) {
    int p_id = place->placeId().place_id;
    auto iter = places_map.find(p_id);

    if (iter == places_map.end()) {
        std::shared_ptr<PlaceData> pd;
        //#pragma omp critical
        //{
        pd = doAddPlace(place, initial_beta, false);
        //}
        double old_beta = pd->beta_;
        pd->doseBeta(dose, source);
        // stats
       Statistics::instance()->recordDosingEvent(p_id, dose, old_beta, pd->beta_);
    } else {
        double old_beta = iter->second->beta_;
        iter->second->doseBeta(dose, source);
        // stats
        Statistics::instance()->recordDosingEvent(p_id, dose, old_beta, iter->second->beta_);
    }
}

bool cmp(const std::pair<int, std::shared_ptr<PlaceData>>  &p1, std::pair<int, std::shared_ptr<PlaceData>> &p2)
{
    return p1.second->beta() < p2.second->beta();
}

using PlaceMapEntryT = std::pair<int, std::shared_ptr<PlaceData>>;


void Places::remove(int count) {

    std::vector<PlaceMapEntryT> vec;
    std::copy_if(places_map.begin(), places_map.end(), std::back_inserter(vec),
            [this](PlaceMapEntryT i) {return !(i.second->place_->isHomeWorkSchool() || i.second->place_ == this->initial_clinic);});
    std::sort(vec.begin(), vec.end(), cmp);
//    for (auto kv : vec) {
//        auto& item = kv.second;
//        std::cout << item->place_->placeId().place_id << ", " << item->beta_ << std::endl;
//
//    }
    for (int i = 0; i < count; ++i) {
        auto& item = vec[i].second;
        int p_id = item->place_->placeId().place_id;
        places_map.erase(p_id);
        // Statistics::instance()->recordDosingEvent(p_id, 0, 0, 0);
        //std::cout << "Removing " << item->place_->placeId().place_id << std::endl;
        --non_perm_count;
        /*
        for (auto code_iter = item->place_->serviceCodesBegin();
                code_iter != item->place_->serviceCodesEnd(); ++code_iter) {
            int code = *code_iter;
            auto& code_vec = code_places_map[code];
            for (auto iter = code_vec.begin(); iter != code_vec.end();) {
                if (iter->get() == item.get()) {
                    iter = code_vec.erase(iter);
                } else {
                    ++iter;
                }
            }
        }
        */
    }
}

void Places::decayBetas(float decay) {
    for (auto kv : places_map) {
        kv.second->decayBeta(decay);
    }

    //Statistics::instance()->initDosingEventRecording(DosingCause::REMOVE);
    int overage = places_map.size() - MAX_PLACES;
    //std::cout << places_map.size() << ", " << overage << std::endl;
    if (overage > 0) {
        if (non_perm_count < overage) {
            throw std::logic_error("Not enough non permanently cached places to remove to remain under MAX_PLACES size");
        }
        remove(overage);
    }
}

double Places::getBeta(int place_id) {
    auto iter = places_map.find(place_id);
    if (iter == places_map.end()) {
        return NA_BETA;
    }

    return iter->second->beta();
}

void Places::fillSendData(CRXPersonData& data) {
    size_t i = 0;
    for (auto& kv : places_map) {
        // place isn't null
        if (kv.second->place_) {
            data.places_place_ids[i] = kv.first;
            data.places_betas[i] = kv.second->beta();
            ++i;
        }
    }

    data.home_id = home_ == nullptr ? -1 : home_->placeId().place_id;
    data.work_id = work_ == nullptr ? -1 : work_->placeId().place_id;
    data.school_id = school_ == nullptr ? -1 : school_->placeId().place_id;
    data.clinic_id = initial_clinic->placeId().place_id;
}

void Places::doSelectPlace(int service_code, const std::string& alpha_service_code, double alpha, ActInfo& current_act,
        std::shared_ptr<Activity> act, std::vector<std::shared_ptr<PlaceData>>& vec) {
    if (vec.size() == 0) {
        return;
    }

    // short circuit scoring if not AB and only one place to choose from
    //auto& vec = iter->second;
    if (act->decision_type != DecisionType::AB && vec.size() == 1) {
        current_act.place = vec[0]->place_;
        current_act.service_code = service_code;
        current_act.atus_code = act->atus_code_;
        return;
    }

    //bool do_log = ((service_code == 5 || service_code == 6 || service_code == 7) && repast::RepastProcess::instance()->getScheduleRunner().currentTick() > 168);

    std::vector<double> scores;
    PlaceScoreCalculator* calc = PlaceScoreCalculator::instance();
    double max_score = -1;
    std::vector<PlaceData*> best_places;
    // if (do_log) {
    //         std::cout << "======" << std::endl;
    // }

    for (auto& data : vec) {
        // if (do_log) {
        //     std::cout << data->place()->placeId().place_id;
        // }
        double score = calc->calculateScore(data->beta_, act->gamma_, service_code,
                current_act.place->location(), data->place_->location());
        
        if (score > max_score) {
            max_score = score;
            best_places.clear();
            best_places.push_back(data.get());
        } else if (score == max_score) {
            best_places.push_back(data.get());
        }

        // if (do_log) {
        //     std::cout << std::endl;
        // }
    }

    repast::shuffleList(best_places);
    if (act->decision_type == DecisionType::AB) {
        int choice = 0;
        // if (do_log) {
        //     std::cout << "max_score: " << max_score << ", alpha: " << alpha << std::endl;
        //     if (max_score > alpha) {
        //         std::cout << "best place: " << best_places[0]->place_->placeId().place_id << std::endl;
        //     }
        //     std::cout << "\n\n";
        // }
        if (max_score > alpha) {
            current_act.place = best_places[0]->place_;
            current_act.service_code = service_code;
            current_act.atus_code = act->atus_code_;
            choice = 1;
        }
        // back out delta -- max_score = beta / (gamma * delta_val);
        double delta = best_places[0]->beta_ / (max_score * act->gamma_);
        Statistics::instance()->recordChoice(max_score, alpha, best_places[0]->beta_, act->gamma_, delta, alpha_service_code, act->atus_code_, choice);
    } else {
        current_act.place = best_places[0]->place_;
        current_act.service_code = service_code;
        current_act.atus_code = act->atus_code_;
    }
}

void Places::doEverywhere(int service_code, ActInfo& current_act, std::shared_ptr<Activity> act) {
    // place doesn't change, but update atus_code and service_code
    current_act.atus_code = act->atus_code_;
    current_act.service_code = service_code;
}

void Places::doHome(int service_code, ActInfo& current_act, std::shared_ptr<Activity> act) {
    current_act.place = home_;
    current_act.service_code = service_code;
    current_act.atus_code = act->atus_code_;
}

void Places::doWork(int service_code, ActInfo& current_act, std::shared_ptr<Activity> act) {
    current_act.place = work_;
    current_act.service_code = service_code;
    current_act.atus_code = act->atus_code_;
}

void Places::doSchool(int service_code, ActInfo& current_act, std::shared_ptr<Activity> act) {
    current_act.place = school_;
    current_act.service_code = SCHOOL_SERVICE_CODE;
    current_act.atus_code = act->atus_code_;
}

void Places::selectPlace(std::shared_ptr<ActT> entry, double alpha, ActInfo& current_act) {
    int service_code = entry->serviceCode();
    std::shared_ptr<Activity> act = entry->act();

    if (service_code == EVERYWHERE_SERVICE_CODE) {
        doEverywhere(service_code, current_act, act);
        return;
    } else if (service_code == HOME_SERVICE_CODE) {
        doHome(service_code, current_act, act);
        return;
    } else if (service_code == WORK_SERVICE_CODE) {
        // some 16 - 30 year olds may be assigned work
        // when in fact they are too young and so have no workplace,
        // so we check for assigned workplace and then send to school if
        // no workplace
        if (work_) {
            doWork(service_code, current_act, act);
        } else if (school_) {
            doSchool(service_code, current_act, act);
        } else {
            doHome(service_code, current_act, act);
        }
        return;

    } else if (service_code == SCHOOL_SERVICE_CODE) {
        // some 16 - 30 year olds may be assigned school
        // when in fact too old for school, so we check if
        // he / she has an assigned school and if not then
        // have them do the "school" activity "everywhere"
        // could be things like on-line courses ...
        if (school_) {
            doSchool(service_code, current_act, act);
        } else {
            doEverywhere(service_code, current_act, act);
        }
        return;
    }


    ServiceCodeMap* sc_map = ServiceCodeMap::instance();
    std::vector<std::shared_ptr<PlaceData>> vec;

    for (int i : sc_map->getPlaces(service_code)) {
        auto iter = places_map.find(i);
        if (iter != places_map.end()) {
            vec.push_back(iter->second);
        }
    }

    doSelectPlace(service_code, entry->alphaServiceCode(), alpha, current_act, act, vec);
}

void Places::reset(std::map<int, std::shared_ptr<CRXPlace>>& place_map, CRXPersonData& data, size_t array_size) {
    clear();
    home_ = data.home_id == NULL_PLACE_ID ? nullptr : place_map[data.home_id].get();
    school_ = data.school_id == NULL_PLACE_ID ? nullptr : place_map[data.school_id].get();
    work_ = data.work_id == NULL_PLACE_ID ? nullptr : place_map[data.work_id].get();
    setInitialClinic(place_map[data.clinic_id]);

    for (size_t i = 0; i < array_size; ++i) {
        int place_id = data.places_place_ids[i];
        if (place_id != PLACE_FILLER) {
            double beta = data.places_betas[i];
            CRXPlace* place = place_map.at(place_id).get();
            doAddPlace(place, beta, true);
        }
    }
}

} /* namespace crx */
