/*
 * BetaRecorder.cpp
 *
 *  Created on: Apr 20, 2018
 *      Author: nick
 */

#include "BetaRecorder.h"

namespace crx {

const std::string BETA_HEADER = "tick,person_id,place_id,beta";

BetaRecorder::BetaRecorder(const std::string& fname, int rank, std::shared_ptr<Filter<unsigned int>> person_filter) :
        betas(std::make_shared<ConcatenatingDataRecorder<BetaScore>>(fname, BETA_HEADER, rank, make_beta_type)),
        filter{person_filter} {}

BetaRecorder::~BetaRecorder() {}

void BetaRecorder::recordBetaScore(double tick, unsigned int person_id, int place_id, double score) {
    if (filter->evaluate(person_id)) {
        betas->record({tick, score, person_id, place_id});
    }
}

void BetaRecorder::write() {
    betas->write(write_beta_type);
    betas->flush();
}

void BetaRecorder::close() {
    betas->close();
}

} /* namespace crx */
