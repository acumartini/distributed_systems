/*
 * utils.h
 *
 *  Created on: Apr 12, 2014
 *      Author: martini
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
#include <algorithm>
#include <numeric>
#include <iterator>
#include <vector>
#include <cmath>


typedef std::vector<double> CreditVec;

namespace utils {

	CreditVec normalize(CreditVec &C);

	CreditVec scale(CreditVec &C, double new_max, double new_min);

	double compute_stdev(CreditVec &C);

} /* namespace utils */

#endif /* UTILS_H_ */
