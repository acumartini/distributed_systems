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
#include <exception>


// global network storage
std::unordered_map<int,std::vector<int> > nodemap;

/*
 * Reads the network from the given file into the global adjecency list unordered map.
 */
void load_network(std::string filename) {
	std::string line;
	//std::ifstream infile(filename);
	std::ifstream infile("data.txt");
	std::vector<int> edges;
	std::vector<int>::size_type sz;

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

				// check for membership and add edge to nodemap according
				try {
				    edges = nodemap[node];
				} catch (std::exception& e) {
				    std::cout << e.what() << '\n';
				    std::cout << "New node: clearing vector to store edge value." << '\n';
				    edges.clear();
				}
				edges.push_back(conn); // record edge
				nodemap[node] = edges;

				// if (node != curr_node) {
				// 	std::cout << "NEW KEY: " << node << '\n';

				// 	// update nodemap
				// 	sz = curr_vec.size();
				// 	nodemap.emplace(curr_node, std::make_pair(sz, curr_vec));

				// 	// reset current node and vector
				// 	curr_node = node;
				// 	curr_vec.clear();

				// 	// record the initial entry to the new edge set
				// 	curr_vec.push_back (conn);
				// } else { 
				// 	// record edge
				// 	curr_vec.push_back (conn);
				// }
			} else {
				std::cout << "Input Error: 2 tokens per line expected." << std::endl;
			}
   		}
   		// final update to nodemap
   		// sz = curr_vec.size();
		// nodemap.emplace(curr_node, std::make_pair(sz, curr_vec));

		// store total node count globally
		// numNodes = curr_no
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

}

int main( int argc , char** argv ) {
	// get cmdline args
	std::string filename = argv[1];
	int numSteps = atoi(argv[2]);

	// intialize adjacency list vector hash
	std::cout << "Loading network edges from " << filename << "..." << std::endl;
	load_network(filename);

	// check hash
	std::cout << "nodemap contains:" << std::endl;
	for (auto& kv: nodemap) {
		std::cout << kv.first << ": ";
		// std::pair<std::vector<int>::size_type, std::vector<int> > p = kv.second;
		std::cout << "size=" << kv.second.size() << std::endl;
		for (auto& edge: kv.second) {
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