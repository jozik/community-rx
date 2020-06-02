/*
 * crx_person_data.h
 *
 *  Created on: May 17, 2018
 *      Author: nick
 */

#ifndef CRX_PERSON_DATA_H_
#define CRX_PERSON_DATA_H_

#include "mpi.h"

#include "constants.h"

namespace crx {

struct CRXPersonData {

    unsigned int person_id;
    // set when person moves to the next place, act type
    int place_id;
    int act_type;

    unsigned int weekend_acts;
    unsigned int weekday_acts;
    double alpha;

    // to recreate ActInfo
    int act_info_place_id;
    unsigned int act_info_atus_code;
    int act_info_service_code;

    // bool 0 or 1
    int switched_place;
    float info_propensity;

    unsigned int weekday_bucket_id, weekend_bucket_id;
    // bool 0 or 1
    int zonked;
    int age_id;

    int work_id;
    int school_id;
    int home_id;
    int clinic_id;

    double places_betas[MAX_PLACES];
    int places_place_ids[MAX_PLACES];
};



void make_mpi_person_type(MPI_Datatype* data_type);

}


#endif /* CRX_PERSON_DATA_H_ */
