/*
 * ResourceVisitRecorder.h
 *
 *  Created on: Apr 3, 2018
 *      Author: nick
 */

#ifndef CHOICERECORDER_H_
#define CHOICERECORDER_H_

#include <set>

#include "Stats.h"
#include "ConcatenatingDataRecorder.h"
#include "Filter.h"

namespace crx {

extern const std::string CHOICE_HEADER;

class ChoiceRecorder {

private:
    std::shared_ptr<ConcatenatingDataRecorder<ChoiceEntry>> choices;
    std::shared_ptr<Filter<std::string>> service_code_filter_;

public:
    ChoiceRecorder(const std::string& fname, int rank, std::shared_ptr<Filter<std::string>> service_code_filter);
    virtual ~ChoiceRecorder();

    void record(double tick, unsigned int person_id, double max_score, double alpha, double beta, double gamma,
        double delta, const std::string& service_code, unsigned int atus_code, int choice);

    void write();
    void close();
};

} /* namespace crx */

#endif /* CHOICERECORDER_H_ */
