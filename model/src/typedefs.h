/*
 * typedefs.h
 *
 *  Created on: Jul 22, 2016
 *      Author: nick
 */

#ifndef TYPEDEFS_H_
#define TYPEDEFS_H_

#include <memory>
#include <utility>

#include "mrsa/Activities.h"

#include "ScheduleEntry.h"

namespace crx {

class CRXPlace;

using ActT = ScheduleEntry;
using ActsT = mrsa::Activities<ActT>;
using ActsPtr = std::shared_ptr<ActsT>;

using PlacePtr = std::shared_ptr<CRXPlace>;
using PID = std::pair<chi_sim::PlaceId, PlacePtr>;

}



#endif /* TYPEDEFS_H_ */
