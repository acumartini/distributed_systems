/* Adam Martini
 * CIS 630 Distributed Systems
 * Project 1
 * rws.cpp
 * 4.20.14
 */

#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
// #include <boost/algorithm/string.hpp>
#include <sstream> 
#include <iterator>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <exception>

#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef std::unordered_set<int> EdgeSet;
typedef std::unordered_map<int, EdgeSet> NodeMap;
typedef std::unordered_map<int, int> IndexMap;
typedef std::pair<int, int> NodePair;
typedef std::vector<double> CreditVec;


// global network storage
// std::unordered_map<int,std::vector<int> > nodemap;
NodeMap nodemap;
IndexMap imap;


// std::vector<std::string> string_split(std::string s, const char delimiter) {
//     size_t start=0;
//     size_t end=s.find_first_of(delimiter);
    
//     std::vector<std::string> output;
    
//     while (end <= std::string::npos) {
// 	    output.emplace_back(s.substr(start, end-start));

// 	    if (end == std::string::npos) 
// 	    	break;

//     	start=end+1;
//     	end = s.find_first_of(delimiter, start);
//     }
    
//     return output;
// }

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

	// std::cout << "source: " << source << std::endl;
	// std::cout << "target: " << target << std::endl;

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
	std::ifstream infile(filename);
	NodePair nodes;
	std::vector<int>::size_type sz;
	EdgeSet edges;
	int line_count = -1;

	if (infile) {
		// get the line entry
		while (getline(infile, line, '\n')) {
			std::cout << "line#" << ++line_count << " : " << line <<std::endl; 

			// split the line into tokens to get the kv pair
    		std::istringstream buf(line);
		    std::istream_iterator<std::string> beg(buf), end;
		    std::vector<std::string> tokens(beg, end); // tokenized!
		    // boost::split(tokens, line, boost::is_any_of(' '));

		    // validate token input size and update nodemap
		    sz = tokens.size();
		    // if (get_nodes(line, nodes) != -1) {
				// convert key to integer
				// int source = nodes.first;
				// int target = nodes.second;
		    if (sz == 2) {
				int source = stoi(tokens[0]);
				int target = stoi(tokens[1]);

				// verify valid edge
				if (source != target) {
					// check if source node is new and update imap
					// if (imap.count(source) == 0) {
					// 	std::cout << "New node: " << source << '\n';
					// 	++node_count;
					// 	imap[source] = node_count;
					// }

					// // check if target node is new and update imap
					// if (imap.count(target) == 0) {
					// 	std::cout << "New node: " << target << '\n';
					// 	++node_count;
					// 	imap[target] = node_count;
					// }

					// update nodemap with new (undirected) edge
					edges = nodemap[source];
					edges.insert(target);
					nodemap[source] = edges;

					edges = nodemap[target];
					edges.insert(source);
					nodemap[target] = edges;
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
	int node, i;

	for (auto& kv: nodemap) {
		sum = 0;
		for (auto& edge: kv.second) {
			i = imap[edge];
			// std::cout << "edge: " << edge << std::endl;
			// std::cout << "credit update: " << C[i] / nodemap[edge].size() << std::endl;
			sum += C[i] / nodemap[edge].size();
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
	// 	EdgeSet es = kv.second;
	// 	std::cout << "index=" << imap[kv.first] << " size=" << es.size() << std::endl;
	// 	for (auto& edge: es) {
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
