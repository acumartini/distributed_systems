/* Adam Martini
 * CIS 630 Distributed Systems
 * Project 1
 * rws.cpp
 * 4.20.14
 */

#include <fstream>
#include <string>
#include <iostream>
#include <sstream> 
#include <iterator>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <exception>
#include <time.h>

typedef std::unordered_set<int> EdgeSet;
typedef std::unordered_map<int, EdgeSet> NodeMap;
typedef std::unordered_map<int, int> IndexMap;


// global network storage
// std::unordered_map<int,std::vector<int> > nodemap;
NodeMap nodemap;
IndexMap imap;

/*
 * Reads the network from the given file into the global adjecency list unordered map.
 */
void load_network(std::string filename) {
	std::string line;
	std::ifstream infile(filename);
	std::vector<int>::size_type sz;
	EdgeSet edges;
	int node_count = -1;

	if (infile) {
		// get the line entry
		while (getline(infile, line, '\n')) {
			// split the line into tokens to get the kv pair
    		std::istringstream buf(line);
		    std::istream_iterator<std::string> beg(buf), end;
		    std::vector<std::string> tokens(beg, end); // tokenized!

		    // validate token input size and update nodemap
		    sz = tokens.size();
		    if (sz == 2) {
				// convert key to integer
				int node = stoi(tokens[0]);
				int conn = stoi(tokens[1]);

				// check for new node
				if (imap.count(node) == 0) {
					std::cout << "New node: " << node << '\n';
					++node_count;
					imap[node] = node_count;
				}

				// update nodemap with new edge
				edges = nodemap[node];
				edges.insert(conn);
				nodemap[node] = edges;

				// check for membership and add edge to nodemap according
				// if (nodemap.count(node) == 0) {
				// 	vp = nodemap[node];
				//     cur_index = vp.first;
				//     edges = vp.second;

				    // edges.insert(conn); // record edge
				    // nodemap[node] = edges;
				// } else {
				    // std::cout << e.what() << '\n';
				    // std::cout << "New node: creating new vector to store edge values." << '\n';
				    // // edges = new std::unordered_set<int>();
				    // ++node_count;
				    // cur_index = node_count;

				    // edges.clear();
				    // edges.insert(conn);
				    // nodemap[node] = edges;
				// }
				// edges.insert(conn); // record edge
				// nodemap[node] = ValuePair(cur_index, edges);
				
			} else {
				std::cout << "Input Error: 2 tokens per line expected." << std::endl;
			}
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
void compute_credit(std::vector<double> &C, std::vector<double> &C_) {
	double sum;

	// for (int i=1; i<=C_.size(); ++i) {
	// 	sum = 0;
	// 	for (auto& edge_node: nodemap[i]) {
	// 		sum += double (C[edge_node]) / double ((nodemap[edge_node].size()));
	// 	}
	// 	std::cout << sum << std::endl;
	// }
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
	std::cout << "nodemap contains:" << std::endl;
	for (auto& kv: nodemap) {
		std::cout << kv.first << ": ";
		EdgeSet es = kv.second;
		std::cout << "index=" << imap[kv.first] << " size=" << es.size() << std::endl;
		for (auto& edge: es) {
			std::cout << edge << ", ";
		}
		std::cout << std::endl;
	}
	
	// compute the normalized credit after numSteps
	std::cout << std::endl;
	std::cout << "Computing the normalized credit vector after " << numSteps << " steps...";
	std::cout << std::endl;
	std::vector<double> credit(nodemap.size(), 1); // initialize credit at t=0 to 1 for each node
	std::vector<double> credit_(nodemap.size(), 0);
	for (int i=0; i<numSteps; ++i) {
		std::cout << "step : " << i << std::endl;
		compute_credit(credit, credit_);
	}

	return 0 ;
}
