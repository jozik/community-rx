/*
 * StatStructs.h
 *
 *  Created on: Dec 15, 2017
 *      Author: nick
 */

#ifndef STATS_H_
#define STATS_H_

#include "mpi.h"

#include "FileOut.h"

namespace crx {


struct BetaScore {

    double tick, beta;
    unsigned int person_id;
    int place_id;
};

//double tick, unsigned int person_id, double max_score, double alpha, double beta, double gamma, int service_code, int choice
struct ChoiceEntry {
    double tick;
    unsigned int person_id;
    double max_score, alpha, beta, gamma, delta;
    // max 3 + null terminator
    char service_code[4];
    unsigned int atus_code;
    int choice;
};

struct ResourceVisit {
    double tick;
    unsigned int person_id;
    int place_id;
    // max 3 + null terminator
    char service_code[4];
    unsigned int atus_code, schedule_id;
};


struct DosingEvent {
    double tick;
    unsigned int person_id;
    int dosing_location;
    int place_id;
    int cause;
    double dose;
    double old_beta;
    double new_beta;
    double threshold;
    int person_count;
};

struct HRXSharedEvent {
    double tick;
    unsigned int p1;
    unsigned int p2;
    int location;
};

struct HRXReceivedEvent {
    double tick;
    unsigned p1;
    int provider_type;
    int location;
};

struct HRXTagCount {
    double tick;
    unsigned int p1;
    int tag_type;
    unsigned count;
};

void make_beta_type(MPI_Datatype* beta_type);
void write_beta_type(FileOut& out, BetaScore& score);

void make_choice_type(MPI_Datatype* choice_type);
void write_choice_type(FileOut& out, ChoiceEntry& score);

void make_resource_vist_type(MPI_Datatype* rv_type);
void write_resource_visit_type(FileOut& out, ResourceVisit& visit);

void make_dosing_event_type(MPI_Datatype* rv_type);
void write_dosing_event(FileOut& out, DosingEvent& evt);

void make_hrx_shared_event_type(MPI_Datatype* rv_type);
void write_hrx_shared_event(FileOut& out, HRXSharedEvent& evt);

void make_hrx_received_event_type(MPI_Datatype* rv_type);
void write_hrx_received_event(FileOut& out, HRXReceivedEvent& evt);

void make_hrx_tag_count_type(MPI_Datatype* rv_type);
void write_hrx_tag_count(FileOut& out, HRXTagCount& htc);



}




#endif /* STATS_H_ */
