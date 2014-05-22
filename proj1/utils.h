/*
 * utils.h
 *
 *  Created on: Apr 12, 2014
 *      Author: martini
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <numeric>
#include <iterator>
#include <vector>
#include <cmath>
#include <time.h>


typedef std::vector<double> CreditVec;

namespace utils {

	bool fexists( const std::string );

	float elapsed_time(clock_t t1, clock_t t2);

	CreditVec normalize(CreditVec &C);

	CreditVec scale(CreditVec &C, double new_max, double new_min);

	double compute_diff_avg(CreditVec &C, CreditVec &C_);

	double compute_stdev(CreditVec &C);

	void save_vector_data(std::vector<double> &data, std::string file);

	void save_results(std::vector<CreditVec> &distribs, std::vector<double> &diff_avg, std::vector<double> &stdevs);

} /* namespace utils */

#endif /* UTILS_H_ */
