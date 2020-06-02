#include "PropensityMap.h"

namespace crx {

PropensityMap::PropensityMap() : distributions() {}

void PropensityMap::addDistribution(unsigned int atus_code, VecOfNormalDistPtr& distribution) {
    distributions.emplace(atus_code, distribution);
}
    
double  PropensityMap::next(unsigned int atus_code, int thread_num) {
    return (*distributions[atus_code])[thread_num]->next();
}


}