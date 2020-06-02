/*
 * crx_person_data.cpp
 *
 *  Created on: May 17, 2018
 *      Author: nick
 */

#include <cstddef>

#include "crx_person_data.h"


namespace crx {

//struct CRXPersonData {
//
//    unsigned int person_id;
//    // set when person moves to the next place, act type
//    int place_id;
//    int act_type;
//
//    unsigned int weekend_acts;
//    unsigned int weekday_acts;
//    double alpha;
//
//    // to recreate ActInfo
//    int act_info_place_id;
//    unsigned int act_info_atus_code;
//    int act_info_service_code;
//
//    // bool 0 or 1
//    int switched_place;
//    float info_propensity;
//
//    unsigned int weekday_bucket_id, weekend_bucket_id;
//    // bool 0 or 1
//    int zonked;
//    int age_id;
//
//    int work_id;
//    int school_id;
//    int home_id;
//    int clinic_id;
//
//    double places_betas[MAX_PLACES];
//    int places_place_ids[MAX_PLACES];
//};


void make_mpi_person_type(MPI_Datatype* data_type) {
    int count = 21;
    MPI_Datatype types[count] = { MPI_UNSIGNED, MPI_INT, MPI_INT,
            MPI_UNSIGNED, MPI_UNSIGNED, MPI_DOUBLE,
            MPI_INT, MPI_UNSIGNED, MPI_INT,
            MPI_INT, MPI_FLOAT,
            MPI_UNSIGNED, MPI_UNSIGNED, MPI_INT, MPI_INT,
            // work, school, etc:
            MPI_INT, MPI_INT, MPI_INT, MPI_INT,
            MPI_DOUBLE, MPI_INT
    };

    int block_lengths[count] = { 1, 1, 1,
            1, 1, 1,
            1, 1, 1,
            1, 1,
            1, 1, 1, 1,
            1, 1, 1, 1,
            MAX_PLACES, MAX_PLACES
    };

    MPI_Aint displacements[count] {
        offsetof(CRXPersonData, person_id),
                offsetof(CRXPersonData, place_id),
                offsetof(CRXPersonData, act_type),
                offsetof(CRXPersonData, weekend_acts),
                offsetof(CRXPersonData, weekday_acts),
                offsetof(CRXPersonData, alpha),
                offsetof(CRXPersonData, act_info_place_id),
                offsetof(CRXPersonData, act_info_atus_code),
                offsetof(CRXPersonData, act_info_service_code),
                offsetof(CRXPersonData, switched_place),
                offsetof(CRXPersonData, info_propensity),
                offsetof(CRXPersonData, weekday_bucket_id),
                offsetof(CRXPersonData, weekend_bucket_id),
                offsetof(CRXPersonData, zonked),
                offsetof(CRXPersonData, age_id),
                offsetof(CRXPersonData, work_id),
                offsetof(CRXPersonData, school_id),
                offsetof(CRXPersonData, home_id),
                offsetof(CRXPersonData, clinic_id),
                offsetof(CRXPersonData, places_betas),
                offsetof(CRXPersonData, places_place_ids)
    };

    MPI_Datatype tmp_type;
    MPI_Type_create_struct(count, block_lengths, displacements, types, &tmp_type);

    MPI_Aint lb, extent;
    MPI_Type_get_extent(tmp_type, &lb, &extent);
    MPI_Type_create_resized(tmp_type, lb, extent, data_type);
    MPI_Type_commit(data_type);
}

}
