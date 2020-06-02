#include "HRXReceivedRecorder.h"

namespace crx {


const std::string HRX_RECEIVED_EVENT_HEADER = "tick,person_id,provider_type,clinic_id";

HRXReceivedRecorder::HRXReceivedRecorder(const std::string& fname, int rank) :
    events(std::make_shared<ConcatenatingDataRecorder<HRXReceivedEvent>>(fname, 
    HRX_RECEIVED_EVENT_HEADER, rank, make_hrx_received_event_type))
{}

HRXReceivedRecorder::~HRXReceivedRecorder() {}

void HRXReceivedRecorder::record(double tick, unsigned int p1, int location, int provider) {
    HRXReceivedEvent evt;
    evt.tick = tick;
    evt.p1 = p1;
    evt.location = location;
    evt.provider_type = provider;
    events->record(evt);
}


void HRXReceivedRecorder::write() {
    events->write(write_hrx_received_event);
    events->flush();
}

void HRXReceivedRecorder::close() {
    events->close();
}

}