/*
 * DosingRecorder.h
 *
 *  Created on: Apr 20, 2018
 *      Author: nick
 */

#ifndef DOSINGRECORDER_H_
#define DOSINGRECORDER_H_

#include "ConcatenatingDataRecorder.h"
#include "Filter.h"
#include "Stats.h"

namespace crx {

extern const std::string DOSING_EVENT_HEADER;

enum class DosingCause {HRX, PEER, USE, DECAY, REMOVE, NONE};

class DosingRecorder {

private:
    std::shared_ptr<ConcatenatingDataRecorder<DosingEvent>> events;
    std::shared_ptr<Filter<unsigned int>> filter;
    std::shared_ptr<Filter<DosingCause>> cause_filter_;

public:
    DosingRecorder(const std::string& fname, int rank, std::shared_ptr<Filter<unsigned int>> person_filter, std::shared_ptr<Filter<DosingCause>> cause_filter);
    virtual ~DosingRecorder();

    void record(double tick, unsigned int person_id, int dosing_location, int place_id, DosingCause cause, double dose, double old_beta, double new_beta,
        double threshold, int person_count);
    void write();
    void close();

};

} /* namespace crx */

#endif /* DOSINGRECORDER_H_ */
