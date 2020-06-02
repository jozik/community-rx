/*
 * ServiceCodeMap.h
 *
 *  Created on: Nov 3, 2017
 *      Author: nick
 */

#ifndef SERVICECODEMAP_H_
#define SERVICECODEMAP_H_

#include <memory>
#include <vector>
#include <map>

#include "Places.h"
#include "constants.h"

namespace crx {

/**
 * Maps places to their service codes, and
 * service codes to the numeric id.
 */
class ServiceCodeMap {

private:
    // place id to the vector of service codes associated with that place
    std::map<int, std::vector<int>> places_service_code;
    // service_code alpha_num id to numeric id
    std::map<std::string, int> service_code_map;
    std::map<int, std::vector<int>> services_places;
    int next_service_code;

    ServiceCodeMap();

    static ServiceCodeMap* instance_;

public:

    static ServiceCodeMap* instance();

    int addServiceCode(const std::string& alpha_service_code);

    void addServiceCode(int place_id, const std::string& alpha_code);

    int getNumericServiceCode(const std::string& alpha_service_code) const;

    const std::vector<int>& getServiceCodes(int place_id) const;

    const std::vector<int>& getPlaces(int service_code) const;

    void reset();

};

} /* namespace crx */

#endif /* SERVICECODEMAP_H_ */
