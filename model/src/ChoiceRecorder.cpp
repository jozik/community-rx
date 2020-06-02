/*
 * ResourceVisitRecorder.cpp
 *
 *  Created on: Apr 3, 2018
 *      Author: nick
 */

#include <cstring>

#include "ChoiceRecorder.h"


namespace crx {

//double tick, unsigned int person_id, double max_score, double alpha, double beta, double gamma, int service_code, int choice
const std::string CHOICE_HEADER = "tick,person_id,max_score,alpha,beta,gamma,delta,service_code,atus_code,choice";

ChoiceRecorder::ChoiceRecorder(const std::string& fname, int rank, std::shared_ptr<Filter<std::string>> service_code_filter) :
        choices(std::make_shared<ConcatenatingDataRecorder<ChoiceEntry>>(fname, CHOICE_HEADER, rank, make_choice_type)),
                service_code_filter_{service_code_filter}  {
}

ChoiceRecorder::~ChoiceRecorder() {
   //close();
}


void ChoiceRecorder::write() {
    choices->write(write_choice_type);
    choices->flush();
}

void ChoiceRecorder::close() {
    choices->close();
}

void ChoiceRecorder::record(double tick, unsigned int person_id, double max_score, double alpha, double beta, double gamma, 
    double delta, const std::string& service_code, unsigned int atus_code, int choice_val) {

    if (service_code_filter_->evaluate(service_code)) {
        ChoiceEntry choice;
        choice.tick = tick;
        choice.person_id = person_id;
        choice.max_score = max_score;
        choice.alpha = alpha;
        choice.beta = beta;
        choice.gamma = gamma;
        choice.delta = delta;
        std::strcpy(choice.service_code, service_code.c_str());
        choice.atus_code = atus_code;
        choice.choice = choice_val;
        choices->record(choice);
    }
}

} /* namespace crx */
