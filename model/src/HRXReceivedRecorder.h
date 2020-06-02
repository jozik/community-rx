/*
 * HRXSharedRecorder.h
 *
 */

#ifndef HRXRECEIVEDRECORDER_H_
#define HRXRECEIVEDRECORDER_H_

#include "ConcatenatingDataRecorder.h"
#include "Stats.h"

namespace crx {

extern const std::string HRX_RECEIVED_EVENT_HEADER;

class HRXReceivedRecorder {

private:
    std::shared_ptr<ConcatenatingDataRecorder<HRXReceivedEvent>> events;

public:
    HRXReceivedRecorder(const std::string& fname, int rank);
    virtual ~HRXReceivedRecorder();

    void record(double tick, unsigned int p1, int location, int provider);
    void write();
    void close();

};

} /* namespace crx */

#endif 