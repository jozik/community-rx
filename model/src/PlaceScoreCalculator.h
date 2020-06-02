/*
 * PlaceScoreCalculator.h
 *
 *  Created on: Dec 4, 2017
 *      Author: nick
 */

#ifndef PLACESCORECALCULATOR_H_
#define PLACESCORECALCULATOR_H_

#include <vector>
#include <map>

#include "GeoPoint.h"
#include "ServiceCodeMap.h"

namespace crx {

enum Delta {LOW, MEDIUM, HIGH};

struct DeltaCalculator {
    double low,med;

    DeltaCalculator(double l, double m);

    Delta operator()(double distane);
};


class PlaceScoreCalculator {

private:
    static PlaceScoreCalculator* instance_;

    std::vector<double> deltas;
    std::map<unsigned int, DeltaCalculator> delta_map;

    PlaceScoreCalculator(double delta_low, double delta_med, double delta_high);

public:
    ~PlaceScoreCalculator();

    static PlaceScoreCalculator* initialize(double delta_low, double delta_med, double delta_high);
    static PlaceScoreCalculator* instance();

    void addDelta(int service_code, double low, double med);

    Delta getDelta(int service_code, double distance);

    double calculateScore(double beta, double gamma, int service_code, const GeoPoint& from, const GeoPoint& to);
};

void initialize_place_score_calculator(const std::string& file, const ServiceCodeMap& code_map, double delta_low, double delta_med,
        double delta_high);

} /* namespace crx */

#endif /* PLACESCORECALCULATOR_H_ */
