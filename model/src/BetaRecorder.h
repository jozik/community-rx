/*
 * BetaRecorder.h
 *
 *  Created on: Apr 20, 2018
 *      Author: nick
 */

#ifndef BETARECORDER_H_
#define BETARECORDER_H_

#include "ConcatenatingDataRecorder.h"
#include "Stats.h"
#include "Filter.h"

namespace crx {

extern const std::string BETA_HEADER;

class BetaRecorder {

private:
    std::shared_ptr<ConcatenatingDataRecorder<BetaScore>> betas;
    std::shared_ptr<Filter<unsigned int>> filter;

public:
    BetaRecorder(const std::string& fname, int rank, std::shared_ptr<Filter<unsigned int>> person_filter);
    virtual ~BetaRecorder();

    void recordBetaScore(double tick, unsigned int person_id, int place_id, double score);
    void write();
    void close();

};

} /* namespace crx */

#endif /* BETARECORDER_H_ */
