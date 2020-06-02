/*
 * PlaceScoreCalculator.cpp
 *
 *  Created on: Dec 4, 2017
 *      Author: nick
 */

#include "chi_sim/CSVReader.h"

#include "PlaceScoreCalculator.h"

namespace crx {

PlaceScoreCalculator* PlaceScoreCalculator::instance_ = nullptr;

DeltaCalculator::DeltaCalculator(double l, double m) : low(l), med(m) {}

Delta DeltaCalculator::operator()(double distance) {
    if (distance <= low) return Delta::LOW;
    if (distance <= med) return Delta::MEDIUM;
    return Delta::HIGH;
}


PlaceScoreCalculator::PlaceScoreCalculator(double delta_low, double delta_med,
        double delta_high) : deltas{delta_low, delta_med, delta_high} {
}

PlaceScoreCalculator::~PlaceScoreCalculator() {
}

double PlaceScoreCalculator::calculateScore(double beta, double gamma, int service_code, const GeoPoint& from, const GeoPoint& to) {
    double distance = from.distance(to);
    Delta delta = getDelta(service_code, distance);
    double delta_val = deltas[delta];

    // bool do_log = ((service_code == 5 || service_code == 6 || service_code == 7) && repast::RepastProcess::instance()->getScheduleRunner().currentTick() > 168);
    // if (do_log) {
    //     cout << ", code: " << service_code << ", beta: " << beta << ", gamma: " << gamma << ", distance: " << distance << ", delta: " << delta << ", delta_val: " << delta_val;
    //     cout << ", score: " << (beta / (gamma * delta_val));
    // }

    return beta / (gamma * delta_val);
}

PlaceScoreCalculator* PlaceScoreCalculator::instance() {
    if (!instance_) {
        throw std::domain_error("PlaceScoreCalculator must be initialized before its used");
    }
    return instance_;
}

PlaceScoreCalculator* PlaceScoreCalculator::initialize(double delta_low, double delta_med, double delta_high) {
    if (instance_) {
        delete instance_;
    }
    instance_ = new PlaceScoreCalculator(delta_low, delta_med, delta_high);
    return instance_;
}

void PlaceScoreCalculator::addDelta(int service_code, double low, double med) {
    delta_map.emplace(service_code, DeltaCalculator{low, med});
}

Delta PlaceScoreCalculator::getDelta(int service_code, double distance) {
    auto iter = delta_map.find(service_code);
    if (iter == delta_map.end()) {
        throw std::invalid_argument("Unknown service code in PlaceScoreCalculator::getDelta");
    }
    DeltaCalculator& calc = iter->second;
    return calc(distance);
}


void initialize_place_score_calculator(const std::string& file, const ServiceCodeMap& code_map, double delta_low, double delta_med,
        double delta_high) {
   chi_sim::CSVReader reader(file);
   std::vector<std::string> line;
   // skip header
   reader.next(line);

   PlaceScoreCalculator* calculator = PlaceScoreCalculator::initialize(delta_low, delta_med, delta_high);
   while (reader.next(line)) {
       // service_code,low,med
       int service_code = code_map.getNumericServiceCode(line[0]);
       double low = std::stod(line[1]);
       double med = std::stod(line[2]);
       calculator->addDelta(service_code, low, med);
   }
}


} /* namespace crx */
