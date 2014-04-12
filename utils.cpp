/*
 * utils.cpp
 *
 *  Created on: Apr 12, 2014
 *      Author: martini
 */

#include "utils.h"

typedef std::vector<double> CreditVec;

namespace utils {

	/*
	 * Returns a normalized copy of the given CreditVec.
	 */
	CreditVec normalize(CreditVec &C) {
		CreditVec C_ = C;
		double sum = std::accumulate(std::begin(C_), std::end(C_), 0.0);
		for (int i=0; i<C_.size(); ++i) {
			C_[i] /= sum;
		}

		return C_;
	}

	/*
	 * Returns a scaled version of the given CreditVec with values ranging between
	 * new_max and new_min.
	 */
	CreditVec scale(CreditVec &C, double &new_max, double &new_min) {
		CreditVec C_ = C;
		double min = *std::min_element(std::begin(C), std::end(C));
		double max = *std::max_element(std::begin(C), std::end(C));

		int i = 0;
		std::for_each (std::begin(C), std::end(C), [&](const double d) {
			C_[i] = (((d - min) / (max - min + 0.0000001)) * (new_max - new_min)) + new_min;
			++i;
		});

		return C_;
	}

	/*
	 * Computes the standard deviation of the values in the given CreditVec.
	 */
	double compute_stdev(CreditVec &C) {
		double sum = std::accumulate(std::begin(C), std::end(C), 0.0);
		double m =  sum / C.size();

		double accum = 0.0;
		std::for_each (std::begin(C), std::end(C), [&](const double d) {
			accum += (d - m) * (d - m);
		});

		return std::sqrt(accum / (C.size()-1));
	}

} /* namespace utils */
