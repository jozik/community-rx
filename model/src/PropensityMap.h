#ifndef SRC_PROPENSITYMAP_H_
#define SRC_PROPENSITYMAP_H_

#include <map>
#include <memory>

#include "CRXRandom.h"

namespace crx {

using VecOfNormalDist = std::vector<std::shared_ptr<NormalDistribution>>;
using VecOfNormalDistPtr = std::shared_ptr<VecOfNormalDist>;

class PropensityMap {

private:
    std::map<unsigned int, VecOfNormalDistPtr> distributions;

public:
    PropensityMap();
    ~PropensityMap() {}

    void addDistribution(unsigned int atus_code, VecOfNormalDistPtr& distribution);
    double next(unsigned int atus_code, int thread_num);

};


}

#endif /*SRC_PROPENSITYMAP_H_*/