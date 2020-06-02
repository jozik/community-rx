
#include <iostream>

#include "CRXRandom.h"

namespace crx {

CRXRandom* CRXRandom::instance_ = nullptr;

CRXRandom::CRXRandom(unsigned int seed, int thread_count) : generators(), unis() {
    //std::cout << "Initializing " << thread_count << " rnd generators" << std::endl;
    for (int i = 0; i < thread_count; ++i) {
        generators.push_back(std::make_shared<std::mt19937>(seed));
        unis.push_back(std::uniform_real_distribution<double>(0, 1.0));
    }
}

CRXRandom::~CRXRandom() {}

void CRXRandom::initialize(unsigned int seed, int thread_count) {
    if (instance_) {
        delete instance_;
    }
    instance_ = new CRXRandom(seed, thread_count);
}

double CRXRandom::next(int thread_number) {
    return unis[thread_number]((*generators[thread_number]));
}

std::shared_ptr<std::mt19937> CRXRandom::engine(int thread_number) {
    return generators[thread_number];
}

CRXRandom* CRXRandom::instance() {
    if (instance_) {
        return instance_;
    } else {
        throw std::invalid_argument("CRXRandom must be initialized before it can be used.");
    }

}

void CRXRandom::createNormalDistributions(double mean, double std_dev, std::shared_ptr<std::vector<std::shared_ptr<NormalDistribution>>>& distributions) {
    for (auto& gen : generators) {
        std::normal_distribution<double> normal = std::normal_distribution<double>(mean, std_dev);
        distributions->push_back(std::make_shared<NormalDistribution>(gen, normal));
    }
}




}