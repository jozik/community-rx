/*
 * Stats.cpp
 *
 *  Created on: Dec 15, 2017
 *      Author: nick
 */

#include "stddef.h"
#include <iostream>

#include "Stats.h"

namespace crx {

void make_beta_type(MPI_Datatype* beta_type) {

    MPI_Datatype types[3] = {MPI_DOUBLE, MPI_UNSIGNED, MPI_INT};
    int block_lengths[3] = {2, 1, 1};
    MPI_Aint displacements[3];
    displacements[0] = offsetof(BetaScore, tick);
    displacements[1] = offsetof(BetaScore, person_id);
    displacements[2] = offsetof(BetaScore, place_id);

    MPI_Datatype tmp_type;
    MPI_Type_create_struct(3, block_lengths, displacements, types, &tmp_type);

    // See https://stackoverflow.com/questions/33618937/trouble-understanding-mpi-type-create-struct
    MPI_Aint lb, extent;
    MPI_Type_get_extent(tmp_type, &lb, &extent );
    MPI_Type_create_resized(tmp_type, lb, extent, beta_type);
    MPI_Type_commit(beta_type);
}

void write_beta_type(FileOut& out, BetaScore& score) {
    out << score.tick << "," << score.person_id << "," << score.place_id
            << "," << score.beta << "\n";
}

void make_choice_type(MPI_Datatype* choice_type) {

    size_t count = 10;
    // tick,person_id,max_score,alpha,beta,gamma,delta,service_code,choice
    MPI_Datatype types[count] = {MPI_DOUBLE, MPI_UNSIGNED, MPI_DOUBLE, MPI_DOUBLE,
        MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_CHAR, MPI_UNSIGNED, MPI_INT};
    int block_lengths[count] = {1, 1, 1, 1, 1, 1, 1, 4, 1, 1};
    MPI_Aint displacements[count];
    displacements[0] = offsetof(ChoiceEntry, tick);
    displacements[1] = offsetof(ChoiceEntry, person_id);
    displacements[2] = offsetof(ChoiceEntry, max_score);
    displacements[3] = offsetof(ChoiceEntry, alpha);
    displacements[4] = offsetof(ChoiceEntry, beta);
    displacements[5] = offsetof(ChoiceEntry, gamma);
    displacements[6] = offsetof(ChoiceEntry, delta);
    displacements[7] = offsetof(ChoiceEntry, service_code);
    displacements[8] = offsetof(ChoiceEntry, atus_code);
    displacements[9] = offsetof(ChoiceEntry, choice);

    MPI_Datatype tmp_type;
    MPI_Type_create_struct(count, block_lengths, displacements, types, &tmp_type);

    MPI_Aint lb, extent;
    MPI_Type_get_extent(tmp_type, &lb, &extent );
    MPI_Type_create_resized(tmp_type, lb, extent, choice_type);
    MPI_Type_commit(choice_type);
}

// tick,person_id,max_score,alpha,beta,gamma,delta,service_code,choice";
void write_choice_type(FileOut& out, ChoiceEntry& entry) {
    out << entry.tick << "," << entry.person_id << ",";
    out << entry.max_score << "," << entry.alpha << "," << entry.beta;
    out << "," << entry.gamma << "," << entry.delta;
    out << "," << entry.service_code << "," << entry.atus_code << "," << entry.choice << "\n";
}

/*
 * struct ResourceVisit {
    double tick;
    unsigned int person_id;
    int place_id;
    char service_code[4];
    unsigned int atus_code;
};
 *
 */

void make_resource_vist_type(MPI_Datatype* rv_type) {
    MPI_Datatype types[6] = {MPI_DOUBLE, MPI_UNSIGNED, MPI_INT, MPI_CHAR, MPI_UNSIGNED, MPI_UNSIGNED};
    int block_lengths[6] = {1, 1, 1, 4, 1, 1};
    MPI_Aint displacements[6];
    displacements[0] = offsetof(ResourceVisit, tick);
    displacements[1] = offsetof(ResourceVisit, person_id);
    displacements[2] = offsetof(ResourceVisit, place_id);
    displacements[3] = offsetof(ResourceVisit, service_code);
    displacements[4] = offsetof(ResourceVisit, atus_code);
    displacements[5] = offsetof(ResourceVisit, schedule_id);

    MPI_Datatype tmp_type;
    MPI_Type_create_struct(6, block_lengths, displacements, types, &tmp_type);

    MPI_Aint lb, extent;
    MPI_Type_get_extent(tmp_type, &lb, &extent );
    MPI_Type_create_resized(tmp_type, lb, extent, rv_type);
    MPI_Type_commit(rv_type);
}

void write_resource_visit_type(FileOut& out, ResourceVisit& visit) {
    out << visit.tick << "," << visit.person_id << "," << visit.place_id << ","
            << visit.service_code << "," << visit.atus_code << "," << visit.schedule_id << "\n";
}


void make_hrx_shared_event_type(MPI_Datatype* rv_type) {
    size_t size = 4;
    MPI_Datatype types[size] = {MPI_DOUBLE, MPI_UNSIGNED, MPI_UNSIGNED, MPI_INT};
    int block_lengths[size] = {1, 1, 1, 1};
    MPI_Aint displacements[size];
    displacements[0] = offsetof(HRXSharedEvent, tick);
    displacements[1] = offsetof(HRXSharedEvent, p1);
    displacements[2] = offsetof(HRXSharedEvent, p2);
    displacements[3] = offsetof(HRXSharedEvent, location);

    MPI_Datatype tmp_type;
    MPI_Type_create_struct(size, block_lengths, displacements, types, &tmp_type);

    MPI_Aint lb, extent;
    MPI_Type_get_extent(tmp_type, &lb, &extent );
    MPI_Type_create_resized(tmp_type, lb, extent, rv_type);
    MPI_Type_commit(rv_type);
}

void write_hrx_shared_event(FileOut& out, HRXSharedEvent& evt) {
    out << evt.tick << "," << evt.p1 << "," << evt.p2 << "," <<
    evt.location << "\n";
}


void make_hrx_received_event_type(MPI_Datatype* rv_type) {
    size_t size = 4;
    MPI_Datatype types[size] = {MPI_DOUBLE, MPI_UNSIGNED, MPI_INT, MPI_INT};
    int block_lengths[size] = {1, 1, 1, 1};
    MPI_Aint displacements[size];
    displacements[0] = offsetof(HRXReceivedEvent, tick);
    displacements[1] = offsetof(HRXReceivedEvent, p1);
    displacements[2] = offsetof(HRXReceivedEvent, provider_type);
    displacements[3] = offsetof(HRXReceivedEvent, location);

    MPI_Datatype tmp_type;
    MPI_Type_create_struct(size, block_lengths, displacements, types, &tmp_type);

    MPI_Aint lb, extent;
    MPI_Type_get_extent(tmp_type, &lb, &extent );
    MPI_Type_create_resized(tmp_type, lb, extent, rv_type);
    MPI_Type_commit(rv_type);
}

void write_hrx_received_event(FileOut& out, HRXReceivedEvent& evt) {
    out << evt.tick << "," << evt.p1 << "," << evt.provider_type << "," <<
    evt.location << "\n";
}

void make_hrx_tag_count_type(MPI_Datatype* rv_type) {
    size_t size = 4;
    MPI_Datatype types[size] = {MPI_DOUBLE, MPI_UNSIGNED, MPI_INT, MPI_UNSIGNED};
    int block_lengths[size] = {1, 1, 1, 1};
    MPI_Aint displacements[size];
    displacements[0] = offsetof(HRXTagCount, tick);
    displacements[1] = offsetof(HRXTagCount, p1);
    displacements[2] = offsetof(HRXTagCount, tag_type);
    displacements[3] = offsetof(HRXTagCount, count);

    MPI_Datatype tmp_type;
    MPI_Type_create_struct(size, block_lengths, displacements, types, &tmp_type);

    MPI_Aint lb, extent;
    MPI_Type_get_extent(tmp_type, &lb, &extent );
    MPI_Type_create_resized(tmp_type, lb, extent, rv_type);
    MPI_Type_commit(rv_type);
}

void write_hrx_tag_count(FileOut& out, HRXTagCount& htc) {
    out << htc.tick << "," << htc.p1 << "," << htc.tag_type << "," <<
    htc.count << "\n";
}

void make_dosing_event_type(MPI_Datatype* evt_type) {
    int count = 10;
    MPI_Datatype types[count] = { MPI_DOUBLE, MPI_UNSIGNED, MPI_INT, MPI_INT, MPI_INT, MPI_DOUBLE,
            MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_INT};
    int block_lengths[count] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    MPI_Aint displacements[count];
    displacements[0] = offsetof(DosingEvent, tick);
    displacements[1] = offsetof(DosingEvent, person_id);
    displacements[2] = offsetof(DosingEvent, dosing_location);
    displacements[3] = offsetof(DosingEvent, place_id);
    displacements[4] = offsetof(DosingEvent, cause);
    displacements[5] = offsetof(DosingEvent, dose);
    displacements[6] = offsetof(DosingEvent, old_beta);
    displacements[7] = offsetof(DosingEvent, new_beta);
    displacements[8] = offsetof(DosingEvent, threshold);
    displacements[9] = offsetof(DosingEvent, person_count);

    MPI_Datatype tmp_type;
    MPI_Type_create_struct(count, block_lengths, displacements, types, &tmp_type);

    MPI_Aint lb, extent;
    MPI_Type_get_extent(tmp_type, &lb, &extent);
    MPI_Type_create_resized(tmp_type, lb, extent, evt_type);
    MPI_Type_commit(evt_type);
}

void write_dosing_event(FileOut& out, DosingEvent& evt) {
    out << evt.tick << "," << evt.person_id << "," << evt.dosing_location << "," << evt.place_id << ","
               << evt.cause << "," << evt.dose << "," << evt.old_beta << "," << evt.new_beta << ","
               << evt.threshold << "," << evt.person_count << "\n";
}


}
