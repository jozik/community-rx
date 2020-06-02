/*
 * Statistics.h
 *
 *  Created on: Feb 29, 2016
 *      Author: nick
 */

#ifndef SRC_STATISTICS_H_
#define SRC_STATISTICS_H_

#include <vector>
#include <set>

#include "Stats.h"
#include "ConcatenatingDataRecorder.h"
#include "ResourceVisitRecorder.h"
#include "DosingRecorder.h"
#include "BetaRecorder.h"
#include "ChoiceRecorder.h"
#include "HRXSharedRecorder.h"
#include "HRXReceivedRecorder.h"
#include "HRXTagCountRecorder.h"
#include "place_source.h"


namespace crx {

class Statistics {

private:
    static Statistics* instance_;

    double tick_;
    unsigned int person_id_;
    int dosing_location;
    DosingCause dosing_cause;
    double threshold_;
    int person_count_;

    ResourceVisitRecorder visit_recorder;
    DosingRecorder dosing_recorder_;
    BetaRecorder beta_recorder_;
    ChoiceRecorder choice_recorder_;
    HRXSharedRecorder hrx_shared_recorder_;
    HRXReceivedRecorder hrx_recv_recorder_;
    HRXTagCountRecorder hrx_tag_count_recorder_;

    Statistics(ChoiceRecorder& choice_recorder, ResourceVisitRecorder& recorder,  
        DosingRecorder& dosing_recorder, BetaRecorder& beta_recorder, HRXSharedRecorder& hrx_shared_recorder,
        HRXReceivedRecorder& hrx_recv_recorder, HRXTagCountRecorder hrx_tag_count_recorder, int rank);

public:
    static Statistics* instance();
    static Statistics* initialize(ChoiceRecorder& choice_recorder, ResourceVisitRecorder& recorder,  
        DosingRecorder& dosing_recorder, BetaRecorder& beta_recorder, HRXSharedRecorder& hrx_shared_recorder, 
        HRXReceivedRecorder& hrx_recv_recorder, HRXTagCountRecorder hrx_tag_count_recorder, int rank);
            
    static void reset();

    virtual ~Statistics();

    void recordBetaScore(double tick, unsigned int person_id, int place_id, double score);
    void recordChoice(double max_score, double alpha, double beta, double gamma, double delta, const std::string& service_code, 
        unsigned int atus_code, int choice);
    void recordVisit(int place_id, int service_code, const std::string& alpha_service_code, unsigned int atus_code, unsigned int schedule_id);

    void writeBetaScores();
    void writeChoices();
    void writeVisits();
    void writeDosingEvents();
    void writeHRXSharedEvents();
    void writeHRXReceivedEvents();
    void writeHRXTagCounts();

    void setTick(double tick);
    void setPersonId(unsigned int person_id);
    void initDosingEventRecording(DosingCause cause);
    void initDosingEventRecording(unsigned int person_id, int location, DosingCause cause, double threshold = 0.0, int person_count = 0);
    void recordDosingEvent(int place_id, double dose, double old_beta, double new_beta);

    void initHRXSharedEventRecording(unsigned int p1, unsigned int p2, int location);
    void recordHRXSharedEvent();
    void recordHRXReceivedEvent(unsigned int p1, int location, int provider);
    void recordHRXTagCount(unsigned int p1, Source source, unsigned int count);
    
};



} /* namespace seir */

#endif /* SRC_STATISTICS_H_ */
