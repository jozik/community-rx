/*
 * DosingRecorder.cpp
 *
 *  Created on: Apr 20, 2018
 *      Author: nick
 */

#include "HRXSharedRecorder.h"

namespace crx {

const std::string HRX_SHARED_EVENT_HEADER = "tick,p1,p2,location";

HRXSharedRecorder::HRXSharedRecorder(const std::string& fname, int rank) :
        events(std::make_shared<ConcatenatingDataRecorder<HRXSharedEvent>>(fname, HRX_SHARED_EVENT_HEADER, rank, 
        make_hrx_shared_event_type)), p1_{0}, p2_{0}, location_{0}
{}
HRXSharedRecorder::~HRXSharedRecorder() {}

void HRXSharedRecorder::init(unsigned int p1, unsigned int p2, int location) {
    p1_ = p1;
    p2_ = p2;
    location_ = location;
}

void HRXSharedRecorder::record(double tick) {
    HRXSharedEvent evt;
    evt.tick = tick;
    evt.p1 = p1_;
    evt.p2 = p2_;
    evt.location = location_;
    events->record(evt);
}

void HRXSharedRecorder::write() {
    events->write(write_hrx_shared_event);
    events->flush();
}

void HRXSharedRecorder::close() {
    events->close();
}

} /* namespace crx */
