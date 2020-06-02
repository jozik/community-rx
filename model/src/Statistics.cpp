/*
 * Statistics.cpp
 *
 *  Created on: Feb 29, 2016
 *      Author: nick
 */

#include "Statistics.h"

namespace crx {

Statistics* Statistics::instance_ = nullptr;

Statistics::Statistics(ChoiceRecorder& choice_recorder, ResourceVisitRecorder& recorder, DosingRecorder& dosing_recorder, 
    BetaRecorder& beta_recorder, HRXSharedRecorder& hrx_shared_recorder, 
    HRXReceivedRecorder& hrx_recv_recorder, HRXTagCountRecorder hrx_tag_count_recorder, int rank) :
        tick_(-1),
        person_id_(0), dosing_location(0), dosing_cause{DosingCause::NONE}, threshold_(0), person_count_(0), visit_recorder(recorder),
        dosing_recorder_(dosing_recorder), beta_recorder_(beta_recorder), choice_recorder_(choice_recorder), 
        hrx_shared_recorder_(hrx_shared_recorder), hrx_recv_recorder_(hrx_recv_recorder), 
        hrx_tag_count_recorder_(hrx_tag_count_recorder)  {
}

Statistics::~Statistics() {
    beta_recorder_.close();
    choice_recorder_.close();
    visit_recorder.close();
    dosing_recorder_.close();
    hrx_shared_recorder_.close();
    hrx_recv_recorder_.close();
    hrx_tag_count_recorder_.close();
}

Statistics* Statistics::initialize(ChoiceRecorder& choice_recorder, ResourceVisitRecorder& recorder,
        DosingRecorder& dosing_recorder, BetaRecorder& beta_recorder, HRXSharedRecorder& hrx_shared_recorder,
        HRXReceivedRecorder& hrx_recv_recorder, HRXTagCountRecorder hrx_tag_count_recorder, int rank) {
    if (instance_) {
        delete instance_;
    }
    instance_ = new Statistics(choice_recorder, recorder, dosing_recorder, beta_recorder, 
        hrx_shared_recorder, hrx_recv_recorder, hrx_tag_count_recorder, rank);
    return instance_;
}

Statistics* Statistics::instance() {
    if (!instance_) {
        throw std::invalid_argument("Statistics must be initialized before it can be used.");
    }
    return instance_;
}


void Statistics::reset() {
    if (!instance_) {
        delete instance_;
        instance_ = nullptr;
    }
}

void Statistics::writeBetaScores() {
    beta_recorder_.write();
}

void Statistics::writeChoices() {
    choice_recorder_.write();
}

void Statistics::writeVisits() {
    visit_recorder.write();
}

void Statistics::writeDosingEvents() {
    dosing_recorder_.write();
}

void Statistics::writeHRXSharedEvents() {
    hrx_shared_recorder_.write();
}

void Statistics::writeHRXReceivedEvents() {
    hrx_recv_recorder_.write();
}

void Statistics::writeHRXTagCounts() {
    hrx_tag_count_recorder_.write();
}

void Statistics::initDosingEventRecording(DosingCause cause) {
    dosing_cause = cause;
    threshold_ = 0;
    person_count_ = 0;
}

void Statistics::initDosingEventRecording(unsigned int person_id, int location, DosingCause cause, double threshold, int person_count) {
    person_id_ = person_id;
    dosing_cause = cause;
    threshold_ = threshold;
    person_count_ = person_count;
    dosing_location = location;
}

void Statistics::recordDosingEvent(int place_id, double dose, double old_beta, double new_beta) {
    dosing_recorder_.record(tick_, person_id_, dosing_location, place_id, dosing_cause, dose, old_beta, new_beta, threshold_, person_count_);
}

void Statistics::recordBetaScore(double tick, unsigned int person_id, int place_id, double score) {
   beta_recorder_.recordBetaScore(tick, person_id, place_id, score);
}

// max_score, alpha, best_places[0]->beta, act->gamma_, service_code, choice
void Statistics::recordChoice(double max_score, double alpha, double beta, double gamma, double delta, const std::string& service_code, 
    unsigned int atus_code, int choice) {
    choice_recorder_.record(tick_, person_id_, max_score, alpha, beta, gamma, delta, service_code, atus_code, choice);
}

void Statistics::recordVisit(int place_id,  int service_code, const std::string& alpha_service_code, unsigned int atus_code, unsigned int schedule_id) {
    visit_recorder.record(tick_, person_id_, place_id, service_code, alpha_service_code, atus_code, schedule_id);
}

void Statistics::setTick(double tick) {
    tick_ = tick;
}

void Statistics::setPersonId(unsigned int person_id) {
    person_id_ = person_id;
}

void Statistics::initHRXSharedEventRecording(unsigned int p1, unsigned int p2, int location) {
    hrx_shared_recorder_.init(p1, p2, location);
}

void Statistics::recordHRXSharedEvent() {
    hrx_shared_recorder_.record(tick_);
}

void Statistics::recordHRXReceivedEvent(unsigned int p1, int location, int provider) {
    hrx_recv_recorder_.record(tick_, p1, location, provider);
}

void Statistics::recordHRXTagCount(unsigned int p1, Source source, unsigned int count) {
    if (count > 0) {
        hrx_tag_count_recorder_.record(tick_, p1, static_cast<int>(source), count);
    }
}


} /* namespace seir */
