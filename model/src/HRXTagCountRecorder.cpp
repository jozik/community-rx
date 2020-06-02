#include "HRXTagCountRecorder.h"

namespace crx {


const std::string HRX_TAG_COUNT_HEADER = "tick,person_id,tag_type,count";

HRXTagCountRecorder::HRXTagCountRecorder(const std::string& fname, int rank) :
    counts(std::make_shared<ConcatenatingDataRecorder<HRXTagCount>>(fname, 
    HRX_TAG_COUNT_HEADER, rank, make_hrx_received_event_type))
{}

HRXTagCountRecorder::~HRXTagCountRecorder() {}

void HRXTagCountRecorder::record(double tick, unsigned int p1, int tag_type, unsigned int count) {
    HRXTagCount htc;
    htc.tick = tick;
    htc.p1 = p1;
    htc.tag_type = tag_type;
    htc.count = count;
    counts->record(htc);
}

void HRXTagCountRecorder::write() {
    counts->write(write_hrx_tag_count);
    counts->flush();
}

void HRXTagCountRecorder::close() {
    counts->close();
}

}