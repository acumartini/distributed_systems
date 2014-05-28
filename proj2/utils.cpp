/*
 * utils.cpp
 *
 *  Created on: Apr 12, 2014
 *      Author: martini
 */

#include "utils.h"

typedef std::vector<double> CreditVec;

namespace utils {

bool fexists ( const std::string filename ) {
  std::ifstream ifile( filename );
  return ifile;
}

/*
 * Outputs the time elapsed between t1 and t2 in seconds to standard output.
 */

float elapsed_time(clock_t t1, clock_t t2) {
    float diff((float)t2-(float)t1);
    return diff / CLOCKS_PER_SEC;
}

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
 * Computes the average squared difference between credit values at C(t-1,i) and C(t,i).
 */
double compute_diff_avg(CreditVec &C, CreditVec &C_) {
	double accum = 0.0;
	for (int i=0; i<C.size(); ++i) {
		accum += std::pow(C[i] - C_[i], 2);
	}

	return std::sqrt(accum / C.size());
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

/*
 * Saves vector of doubles as a .dat file.
 */
void save_vector_data(std::vector<double> &data, std::string file) {
	std::ofstream myfile;
 	myfile.open (file);
 	for (auto& d: data) {
 		myfile << d << '\n';
 	}
  	myfile.close();
}

/*
 * Saves the distribution vectors and their standard deviations as .dat files.
 */
void save_results(std::vector<CreditVec> &distribs, std::vector<double> &diff_avg, std::vector<double> &stdevs) {
	int i = 0;
	for (auto& cv: distribs) {
		save_vector_data(cv, "data/distrib" + std::to_string(++i) + ".dat");
	}
	save_vector_data(diff_avg, "data/diffavg.dat");
	save_vector_data(stdevs, "data/stdev.dat");
}

} /* namespace utils */
