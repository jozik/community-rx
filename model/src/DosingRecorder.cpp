/*
 * DosingRecorder.cpp
 *
 *  Created on: Apr 20, 2018
 *      Author: nick
 */

#include "DosingRecorder.h"

namespace crx {

const std::string DOSING_EVENT_HEADER = "tick,person_id,dosing_location_id,place_id,cause,dose,old_beta,new_beta,threshold,person_count";

DosingRecorder::DosingRecorder(const std::string& fname, int rank,
        std::shared_ptr<Filter<unsigned int>> person_filter, std::shared_ptr<Filter<DosingCause>> cause_filter) :
        events(std::make_shared<ConcatenatingDataRecorder<DosingEvent>>(fname, DOSING_EVENT_HEADER, rank, make_dosing_event_type)), filter{person_filter},
        cause_filter_{cause_filter} {

}
DosingRecorder::~DosingRecorder() {}

void DosingRecorder::record(double tick, unsigned int person_id, int dosing_location, int place_id, DosingCause cause, double dose, double old_beta, double new_beta, double threshold, int person_count) {
    if (filter->evaluate(person_id) && cause_filter_->evaluate(cause)) {
        DosingEvent evt;
        evt.tick = tick;
        evt.person_id = person_id;
        evt.dosing_location = dosing_location;
        evt.place_id = place_id;
        evt.cause = static_cast<int>(cause);
        evt.dose = dose;
        evt.old_beta = old_beta;
        evt.new_beta = new_beta;
        evt.threshold = threshold;
        evt.person_count = person_count;
        events->record(evt);
    }
}

void DosingRecorder::write() {
    events->write(write_dosing_event);
    events->flush();
}

void DosingRecorder::close() {
    events->close();
}

} /* namespace crx */
