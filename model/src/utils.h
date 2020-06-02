/*
 * utils.h
 *
 *  Created on: Jan 12, 2018
 *      Author: nick
 */

#ifndef UTILS_H_
#define UTILS_H_

#include "boost/random/variate_generator.hpp"
#include "boost/random/normal_distribution.hpp"
#include "boost/random/mersenne_twister.hpp"


namespace crx {

using NormalGenerator = boost::variate_generator<boost::mt19937&, boost::normal_distribution<>>;

std::string get_mem();

void process_mem_usage(double& vm_usage, double& resident_set);

}




#endif /* UTILS_H_ */
