/*
 * ResourceVisitRecorder.cpp
 *
 *  Created on: Apr 3, 2018
 *      Author: nick
 */

#include <cstring>

#include "ResourceVisitRecorder.h"


namespace crx {

const std::string RESOURCE_VISIT_HEADER = "tick,person_id,place_id,service_code,atus_code,schedule_id";

ResourceVisitRecorder::ResourceVisitRecorder(const std::string& fname, int rank, std::shared_ptr<Filter<std::string>> service_code_filter,
            std::shared_ptr<Filter<unsigned int>> atus_code_filter, std::shared_ptr<Filter<unsigned int>> person_filter, 
            std::shared_ptr<Filter<int>> place_filter) :
        visits(std::make_shared<ConcatenatingDataRecorder<ResourceVisit>>(fname, RESOURCE_VISIT_HEADER, rank, make_resource_vist_type)),
                service_code_filter_{service_code_filter}, atus_code_filter_{atus_code_filter}, person_filter_{person_filter}, place_filter_{place_filter} {
}

ResourceVisitRecorder::~ResourceVisitRecorder() {
   //close();
}


void ResourceVisitRecorder::write() {
    visits->write(write_resource_visit_type);
    visits->flush();
}

void ResourceVisitRecorder::close() {
    visits->close();
}

void ResourceVisitRecorder::record(double tick, unsigned int person_id, int place_id, int service_code,
       const std::string& alpha_service_code, unsigned int atus_code, unsigned int schedule_id) {
    if (person_filter_->evaluate(person_id) && service_code_filter_->evaluate(alpha_service_code) && 
            atus_code_filter_->evaluate(atus_code) && place_filter_->evaluate(place_id)) {
        ResourceVisit visit;
        visit.tick = tick;
        visit.person_id = person_id;
        visit.place_id = place_id;
        visit.atus_code = atus_code;
        //if (alpha_service_code.length() > 3) {
        //    std::cout << "what!!! " <<  alpha_service_code << std::endl;
        //}
        std::strcpy(visit.service_code, alpha_service_code.c_str());
        visit.schedule_id = schedule_id;
        visits->record(visit);
    }
}

} /* namespace crx */
