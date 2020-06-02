#ifndef SRC_CRXRANDOM_H_
#define SRC_CRXRANDOM_H_

#include <random>
#include <vector>
#include <memory>

namespace crx {


template<typename Dist>
class Rnd {

private:
    std::shared_ptr<std::mt19937> generator;
    Dist distribution;

public:
    Rnd(std::shared_ptr<std::mt19937> gen, Dist dist);
    ~Rnd();

    double next();
};

template<typename Dist>
Rnd<Dist>::Rnd(std::shared_ptr<std::mt19937> gen, Dist dist) : generator(gen) , distribution(dist) {}

template<typename Dist>
Rnd<Dist>::~Rnd() {}

template<typename Dist>
double Rnd<Dist>::next() {
    return distribution((*generator));
}

using NormalDistribution = Rnd<std::normal_distribution<double>>;


class CRXRandom {

private:
    static CRXRandom* instance_;

    std::vector<std::shared_ptr<std::mt19937>> generators;
    std::vector<std::uniform_real_distribution<double>> unis;
    int thread_count_;

    CRXRandom(unsigned int seed, int thread_count);

public:
    ~CRXRandom();
    static void initialize(unsigned int seed, int num_threads);
    static CRXRandom* instance();

    double next(int thread_num);
    std::shared_ptr<std::mt19937> engine(int thread_number);
    void createNormalDistributions(double mean, double std_dev, 
         std::shared_ptr<std::vector<std::shared_ptr<NormalDistribution>>>& distributions);
    
};


}

#endif /*SRC_CRXRANDOM_H_*/