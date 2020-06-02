/*
 * HRXTagCountRecorder.h
 */

#ifndef HRXTAGCOUNTRECORDER_H_
#define HRXTAGCOUNTRECORDER_H_

#include "ConcatenatingDataRecorder.h"
#include "Stats.h"

namespace crx {

extern const std::string HRX_TAG_COUNT_HEADER;

class HRXTagCountRecorder {

private:
    std::shared_ptr<ConcatenatingDataRecorder<HRXTagCount>> counts;

public:
    HRXTagCountRecorder(const std::string& fname, int rank);
    virtual ~HRXTagCountRecorder();

    void record(double tick, unsigned int p1, int tag_type, unsigned int count);
    void write();
    void close();

};

} /* namespace crx */

#endif 