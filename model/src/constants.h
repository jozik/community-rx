/*
 * constants.h
 *
 *  Created on: Nov 1, 2017
 *      Author: nick
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <string>

namespace crx {

#ifdef UNIT_TESTS
    const int MAX_PLACES = 9;
#else
    const int MAX_PLACES = 200;
#endif

const int PLACE_FILLER = -1;
const double INIT_BETA = 0.02;

const int HOME_SERVICE_CODE = 0;
const int SCHOOL_SERVICE_CODE = 1;
const int WORK_SERVICE_CODE = 2;
const int ZONKED_SERVICE_CODE = 3;
const int EVERYWHERE_SERVICE_CODE = 4;
const int CLINIC_CODE_1 = 5;
const int CLINIC_CODE_2 = 6;
const int CLINIC_CODE_3 = 7;
const int CLINIC_CODE_4 = 8;
const int CLINIC_CODE_5 = 9;
const int NEXT_SERVICE_CODE = 10;

const std::string HOME_SERVICE_CODE_ALPHA = "H";
const std::string SCHOOL_SERVICE_CODE_ALPHA = "S";
const std::string WORK_SERVICE_CODE_ALPHA = "W";
const std::string CLINIC_SERVICE_CODE_ALPHA = "C";
const std::string ZONKED_SERVICE_CODE_ALPHA = "Z";
const std::string EVERYWHERE_SERVICE_CODE_ALPHA = "E";

const std::string CLINIC_CODE_1_ALPHA = "G05";
const std::string CLINIC_CODE_2_ALPHA = "G12";
const std::string CLINIC_CODE_3_ALPHA = "G14";
const std::string CLINIC_CODE_4_ALPHA = "G02";
const std::string CLINIC_CODE_5_ALPHA = "G07";

const std::string WORKPLACE_TYPE = "workplace";
const std::string SCHOOL_TYPE = "school";
const std::string HOUSEHOLD_TYPE = "household";
const std::string CLINIC_TYPE = "clinic";


}

#endif /* CONSTANTS_H_ */
