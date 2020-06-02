/*
 * run.h
 *
 *  Created on: Apr 26, 2018
 *      Author: nick
 */

#ifndef RUN_H_
#define RUN_H_

#include <string>

#include "mpi.h"

#include "repast_hpc/Properties.h"

namespace crx {

/**
 * Runs the model given a properties file.
 */
void run_model(repast::Properties& props);

/**
 * Runs the model using the specified configuration file and parameters.
 *
 * @param props the repast hpc configuration file
 * @param parameters a tab separated list of parameters where each parameter
 * is a key value pair separated by an "=".
 */
// should be run from swift via an @location

std::string crx_model_run(MPI_Comm comm, const std::string& props_file, const std::string& parameters);

}



#endif /* RUN_H_ */
