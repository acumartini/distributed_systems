/* Adam Martini
 * CIS 630 Distributed Systems
 * Project 1
 * rws.cpp
 * 4.20.14
 */

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <time.h>
#include <assert.h>

#include "utils.h"

typedef std::unordered_set<int> EdgeSet;
typedef std::unordered_map<int, EdgeSet*> NodeMap;
typedef std::unordered_map<int, int> IndexMap;
typedef std::pair<int, int> NodePair;
typedef std::vector<double> CreditVec;

// global network storage
NodeMap nodemap;
IndexMap imap;


int get_nodes(const std::string &line, NodePair &nodes) {
	std::string source, target;
	int i = 0, j = 0;
	char c = line[j];

	// eat white space
	while (!isdigit(c)) { c = line[++j]; }
	i = j;
	// get source node
	while (isdigit(c)) { c = line[++j]; }
	source = line.substr(i, j);
	i = j;
	// eat white space
	while (!isdigit(c)) { c = line[++j]; }
	i = j;
	// get target node
	while (isdigit(c)) { c = line[++j]; }
	target = line.substr(i, j);

	// test for empty strings
	if (source.length() == 0 || target.length() == 0) { return -1; }

	// convert node string to integers
	nodes.first = stoi(source);
	nodes.second = stoi(target);

	return 0;
}

/*
 * Reads the network from the given file into the global adjecency list unordered map.
 */
void load_network(std::string filename) {
	std::string line;
	NodePair nodes;
	NodeMap::const_iterator got;
	EdgeSet *edges;
	int line_count = -1, output_mod = 100000;

	// load file and iterate through each line of input
	std::ifstream infile(filename);
	if (infile) {
		while (getline(infile, line, '\n')) {
			// output line_count every output_mod lines
			++line_count;
			if (line_count % output_mod == 0)
				std::cout << "line#" << line_count << std::endl;

		    // convert key to integer and update nodemap with valid input
		    if (get_nodes(line, nodes) != -1) {
				int source = nodes.first;
				int target = nodes.second;

				// verify valid edge
				if (source != target) {
					// update nodemap with new (undirected) edge
					got = nodemap.find(source);
					if (got == nodemap.end()) { // create a new entry
						nodemap[source] = new EdgeSet();
						edges = nodemap[source];
					} else {
						edges = got->second;
					}
					edges->insert(target);

					got = nodemap.find(target);
					if (got == nodemap.end()) { // create a new entry
						nodemap[target] = new EdgeSet();
						edges = nodemap[target];
					} else {
						edges = got->second;
					}
					edges->insert(source);
				}
			} else {
				std::cout << "Input Error: 2 tokens per line expected." << std::endl;
			}
   		}
   		// populate imap
   		int i = -1;
   		for (auto& kv: nodemap) {
   			imap[kv.first] = ++i;
   		}
	}
	infile.close( );
}

/*
 * Computes a credit update for the next time step given the current credit values for 
 * each node in the network.
 * 
 * @params: C - stores the current credit values for each node in the network
 * 			C_ - stores the credit values for each node at time step t+1
 */
void credit_update (CreditVec &C, CreditVec &C_) {
	double sum;
	int i;

	// compute credit for the next time step
	for (auto& kv: nodemap) {
		sum = 0;
		for (auto& edge: (*kv.second)) {
			i = imap[edge];
			sum += C[i] / nodemap[edge]->size();
		}
		i = imap[kv.first];
		C_[i] = sum;
	}
}


int main ( int argc , char** argv ) {
	clock_t t1,t2;

	// get cmdline args
	std::string filename = argv[1];
	int num_steps = atoi(argv[2]);

	// initialize adjacency list vector hash
	std::cout << "Loading network edges from " << filename << "..." << std::endl;
	t1=clock();
	load_network(filename);
	t2=clock();
	utils::output_elapsed_time(t1, t2);
	
	// compute the normalized credit after numSteps
	std::cout << std::endl;
	std::cout << "Computing the normalized credit vector after " << num_steps << " steps...";
	std::cout << std::endl;
	CreditVec C(nodemap.size(), 1); // initialize credit at t=0 to 1 for each node
	CreditVec C_(nodemap.size(), 0);
	CreditVec Cnorm;
	std::vector<CreditVec> distribs(num_steps);
	std::vector<double> diff_avg(num_steps);
	std::vector<double> stdevs(num_steps);
	double max = 1.0, min = -1.0;

	for (int i=0; i<num_steps; ++i) {
		std::cout << "step : " << i << std::endl;
		credit_update(C, C_);

		// compute and store the average squared difference between C(t-1,i) and C(t, i)
		// diff_avg[i] = utils::compute_diff_avg(C, C_);

		// normalize C and store distribution convergence for plotting
		// Cnorm = utils::normalize(C);
		// distribs[i] = Cnorm;

		// compute stdev and store for plotting
		// stdevs[i] = utils::compute_stdev(Cnorm);

		C = C_; // C(t+1) becomes C(t) for next iteration
	}

	utils::save_results(distribs, diff_avg, stdevs);

	// free heap memory
	for (auto& kv: nodemap) {
		delete kv.second;
	}

	return 0 ;
}
