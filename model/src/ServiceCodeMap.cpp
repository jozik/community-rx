/*
 * ServiceCodeMap.cpp
 *
 *  Created on: Nov 3, 2017
 *      Author: nick
 */

#include <exception>
#include <iostream>

#include "ServiceCodeMap.h"

namespace crx {

ServiceCodeMap* ServiceCodeMap::instance_ = new ServiceCodeMap();

ServiceCodeMap* ServiceCodeMap::instance() {
    return instance_;
}

ServiceCodeMap::ServiceCodeMap() : places_service_code{}, service_code_map{}, services_places{}, next_service_code{NEXT_SERVICE_CODE} {
    reset();
}

void ServiceCodeMap::reset() {
    places_service_code.clear();
    service_code_map.clear();
    services_places.clear();
    next_service_code = NEXT_SERVICE_CODE;

    service_code_map.emplace(HOME_SERVICE_CODE_ALPHA, HOME_SERVICE_CODE);
    service_code_map.emplace(SCHOOL_SERVICE_CODE_ALPHA, SCHOOL_SERVICE_CODE);
    service_code_map.emplace(WORK_SERVICE_CODE_ALPHA, WORK_SERVICE_CODE);
    service_code_map.emplace(ZONKED_SERVICE_CODE_ALPHA, ZONKED_SERVICE_CODE);
    service_code_map.emplace(EVERYWHERE_SERVICE_CODE_ALPHA, EVERYWHERE_SERVICE_CODE);

    service_code_map.emplace(CLINIC_CODE_1_ALPHA, CLINIC_CODE_1);
    service_code_map.emplace(CLINIC_CODE_2_ALPHA, CLINIC_CODE_2);
    service_code_map.emplace(CLINIC_CODE_3_ALPHA, CLINIC_CODE_3);
    service_code_map.emplace(CLINIC_CODE_4_ALPHA, CLINIC_CODE_4);
    service_code_map.emplace(CLINIC_CODE_5_ALPHA, CLINIC_CODE_5);
}

int ServiceCodeMap::addServiceCode(const std::string& alpha_service_code) {
    auto iter = service_code_map.find(alpha_service_code);
    int service_code = next_service_code;
    if (iter == service_code_map.end()) {
          service_code_map.emplace(alpha_service_code, service_code);
          ++next_service_code;
          return service_code;
    } else {
        return iter->second;
    }
}

void ServiceCodeMap::addServiceCode(int place_id, const std::string& alpha_service_code) {
   int service_code = addServiceCode(alpha_service_code);

   auto p_iter = places_service_code.find(place_id);
   if (p_iter == places_service_code.end()) {
       std::vector<int> codes{service_code};
       places_service_code.emplace(place_id, codes);
   } else {
       p_iter->second.push_back(service_code);
   }

   if (service_code != HOME_SERVICE_CODE && service_code != WORK_SERVICE_CODE && service_code != ZONKED_SERVICE_CODE &&
           service_code != SCHOOL_SERVICE_CODE && service_code != EVERYWHERE_SERVICE_CODE) {
       auto v_iter = services_places.find(service_code);
       if (v_iter == services_places.end()) {
           services_places.emplace(service_code, std::vector<int>{place_id});
       } else {
           v_iter->second.push_back(place_id);
       }
   }
}

const std::vector<int>& ServiceCodeMap::getServiceCodes(int place_id) const {
    auto iter = places_service_code.find(place_id);
    if (iter == places_service_code.end()) {
       throw std::invalid_argument("Place id " + std::to_string(place_id) + " not found in service code map");
    }
    return iter->second;
}

const std::vector<int>& ServiceCodeMap::getPlaces(int service_code) const {
    auto iter = services_places.find(service_code);
    if (iter == services_places.end()) {
        //std::cout << services_places.size() << std::endl;
        //for (auto kv : services_places) {
        //    std::cout << kv.first << std::endl;
        //}
        throw std::invalid_argument(
                "Service code " + std::to_string(service_code) + " not found in service code map");
    }
    return iter->second;
}

int ServiceCodeMap::getNumericServiceCode(const std::string& alpha_service_code) const {
    //std::cout << "-" << alpha_service_code << "-" << std::endl;
    auto iter = service_code_map.find(alpha_service_code);
    if (iter == service_code_map.end()) {
        throw std::invalid_argument(alpha_service_code + " not found in service code map");
    }

    return iter->second;


}

} /* namespace crx */
