/*
 * HRXSharedRecorder.h
 *
 *  Created on: Apr 20, 2018
 *      Author: nick
 */

#ifndef HRXSHAREDRECORDER_H_
#define HRXSHAREDRECORDER_H_

#include "ConcatenatingDataRecorder.h"
#include "Stats.h"

namespace crx {

extern const std::string HRX_SHARED_EVENT_HEADER;


class HRXSharedRecorder {

private:
    std::shared_ptr<ConcatenatingDataRecorder<HRXSharedEvent>> events;
    unsigned int p1_, p2_;
    int location_;

public:
    HRXSharedRecorder(const std::string& fname, int rank);
    virtual ~HRXSharedRecorder();

    void init(unsigned int p1, unsigned int p2, int location);
    void record(double tick);
    void write();
    void close();

};

} /* namespace crx */

#endif 
