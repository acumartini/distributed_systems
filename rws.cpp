/* Adam Martini
 * CIS 630 Distributed Systems
 * Project 1
 * rws.cpp
 * 4.20.14
 */

#include <fstream>
#include <string>
#include <iostream>
// #include <algorithm>
// #include <boost/algorithm/string.hpp>
// #include <sstream> 
#include <iterator>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <exception>

#include <time.h>
#include <assert.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <ctype.h>

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
 * Normalizes CreditVec C in place.
 */
void normalize(CreditVec &C) {
	double sum = 0;
	for (auto& credit: C) {
		sum += credit;
	}
	for (int i=0; i<C.size(); ++i) {
		C[i] /= sum;
	}
}

/*
 * Computes a credit update for the next time step given the current credit values for 
 * each node in the network.
 * 
 * @params: C - stores the current credit values for each node in the network
 * 			C_ - stores the credit values for each node at time step t+1
 */
void compute_credit(CreditVec &C, CreditVec &C_) {
	double sum;
	int i;

	for (auto& kv: nodemap) {
		sum = 0;
		for (auto& edge: (*kv.second)) {
			i = imap[edge];
			// std::cout << "edge: " << edge << std::endl;
			// std::cout << "credit update: " << C[i] / nodemap[edge].size() << std::endl;
			sum += C[i] / nodemap[edge]->size();
		}
		// std::cout << sum << std::endl;
		i = imap[kv.first];
		C_[i] = sum;
	}

	// noramlize credit vector
	normalize(C_);
	C = C_;
}

int main( int argc , char** argv ) {
	std::clock_t t1,t2;

	// get cmdline args
	std::string filename = argv[1];
	int numSteps = atoi(argv[2]);

	// initialize adjacency list vector hash
	std::cout << "Loading network edges from " << filename << "..." << std::endl;
	t1=clock();
	load_network(filename);
	t2=clock();
    float diff((float)t2-(float)t1);
    float seconds = diff / CLOCKS_PER_SEC;
    std::cout << "time: " << seconds << std::endl;

	// check hash
	// std::cout << "nodemap contains:" << std::endl;
	// for (auto& kv: nodemap) {
	// 	std::cout << kv.first << ": ";
	// 	EdgeSet *es = kv.second;
	// 	std::cout << "index=" << imap[kv.first] << " size=" << es->size() << std::endl;
	// 	for (auto& edge: (*es)) {
	// 		std::cout << edge << " ";
	// 	}
	// 	std::cout << std::endl;
	// }
	
	// compute the normalized credit after numSteps
	std::cout << std::endl;
	std::cout << "Computing the normalized credit vector after " << numSteps << " steps...";
	std::cout << std::endl;
	CreditVec C(nodemap.size(), 1); // initialize credit at t=0 to 1 for each node
	CreditVec C_(nodemap.size(), 0);

	for (int i=0; i<numSteps; ++i) {
		std::cout << "step : " << i << std::endl;
		compute_credit(C, C_);
	}

	// check final credit
	for (auto& credit: C) {
		std::cout << credit << std::endl;
	}
	std::cout << "C.size()=" << C.size() << std::endl;

	return 0 ;
}
