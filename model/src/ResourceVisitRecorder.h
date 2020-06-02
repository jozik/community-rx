/*
 * ResourceVisitRecorder.h
 *
 *  Created on: Apr 3, 2018
 *      Author: nick
 */

#ifndef RESOURCEVISITRECORDER_H_
#define RESOURCEVISITRECORDER_H_

#include <set>

#include "Stats.h"
#include "ConcatenatingDataRecorder.h"
#include "Filter.h"

namespace crx {

extern const std::string RESOURCE_VISIT_HEADER;

class ResourceVisitRecorder {

private:
    std::shared_ptr<ConcatenatingDataRecorder<ResourceVisit>> visits;
    std::shared_ptr<Filter<std::string>> service_code_filter_;
    std::shared_ptr<Filter<unsigned int>> atus_code_filter_;
    std::shared_ptr<Filter<unsigned int>> person_filter_;
    std::shared_ptr<Filter<int>> place_filter_;

public:
    ResourceVisitRecorder(const std::string& fname, int rank, std::shared_ptr<Filter<std::string>> service_code_filter,
            std::shared_ptr<Filter<unsigned int>> atus_code_filter, std::shared_ptr<Filter<unsigned int>> person_filter,
            std::shared_ptr<Filter<int>> place_filter);
    virtual ~ResourceVisitRecorder();

    void record(double tick, unsigned int person_id, int place_id, int service_code,
            const std::string& alpha_service_code, unsigned int atus_code, unsigned int schedule_id);

    void write();
    void close();
};

} /* namespace crx */

#endif /* RESOURCEVISITRECORDER_H_ */
