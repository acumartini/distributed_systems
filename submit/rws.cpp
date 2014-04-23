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
#include <stdio.h>
#include <time.h>
#include <assert.h>

#include "utils.h"

typedef unsigned long GraphSize;
typedef std::unordered_set<GraphSize> EdgeSet;
typedef std::unordered_map<GraphSize, EdgeSet*> NodeMap;
typedef std::vector<GraphSize> KeyList;
typedef std::unordered_map<GraphSize, GraphSize> IndexMap;
typedef std::pair<GraphSize, GraphSize> NodePair;
typedef std::vector<double> CreditVec;

// global network storage
NodeMap nodemap;
KeyList keys;
IndexMap imap;


/*
 * Parse a line of network data input (an edge) into a source/target node integer pair.
 */
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
 * Reads the network from the given file into the global adjacency list unordered map.
 */
void load_network(std::string filename) {
	std::string line;
	NodePair nodes;
	NodeMap::const_iterator got;
	EdgeSet *edges;
	// GraphSize line_count = 0, output_mod = 1000000;

	// load file and iterate through each line of input
	std::ifstream infile(filename);
	if (infile) {
		while (getline(infile, line, '\n')) {
			// output line_count every output_mod lines
			// if (line_count != 0 && line_count % output_mod == 0)
			// 	printf( "line#%lu\n", line_count );
			// line_count++;

		    // convert key to integer and update nodemap with valid input
		    if (get_nodes(line, nodes) != -1) {
				GraphSize source = nodes.first;
				GraphSize target = nodes.second;

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
   		// populate keylist and imap
   		keys.reserve( nodemap.size() );
   		GraphSize i = 0;
   		for (auto& kv: nodemap) {
   			keys.push_back( kv.first );
   			imap[kv.first] = i++;
   		}
	}
	infile.close( );
}

/*
 * Computes a credit update for the next time step given the current credit values for 
 * each node in the network.  Uses paralellism by mapping updates to available cores
 * through the OpenMP framework.
 * 
 * @params: C - stores the current credit values for each node in the network
 * 			C_ - stores the credit values for each node at time step t+1
 */
void credit_update (CreditVec &C, CreditVec &C_) {
	double sum;
	GraphSize i;

	// compute credit for the next time step
	#pragma omp parallel for private( i, sum ) shared( C, C_ )
	for (int j=0; j<keys.size(); ++j ) {
		EdgeSet *edges = nodemap[keys[j]];
		sum = 0;
		for (auto& edge: *edges) {
			i = imap[edge];
			sum += C[i] / nodemap[edge]->size();
		}
		i = imap[keys[j]];
		C_[i] = sum;
	}
}

/*
 * Writes the network and random walk information to the given file.
 */
void write_output ( std::string filename, std::vector<CreditVec> updates ) {
	FILE * pfile = fopen ( filename.c_str(), "w" );

	for ( GraphSize n=1; n<updates[0].size()+1; ++n ) {
		fprintf( pfile, "%lu\t%lu", n, nodemap[n] ? nodemap[n]->size() : 0 );
		// output update values for node n
		for (int i=0; i<updates.size(); ++i) {
			fprintf( pfile, "\t%.6lf", updates[i][imap[n]] );
		}
		fprintf( pfile, "\n" );
	}

	fclose( pfile );
}


int main ( int argc , char** argv ) {
	clock_t t1,t2;

	// get cmdline args
	std::string input_file = argv[1];
	std::string output_file = argv[2];
	int num_steps = atoi( argv[3] );

	// initialize adjacency list vector hash
	printf("Loading network edges from %s\n", input_file.c_str());
	t1=clock();
	load_network(input_file);
	t2=clock();
	printf( "Time to read input file = %f seconds\n", utils::elapsed_time(t1, t2) );
	
	// compute the normalized credit after numSteps
	printf("\nComputing the Credit Values for %d Rounds:\n", num_steps);
	CreditVec C(nodemap.size(), 1); // initialize credit at t=0 to 1 for each node
	CreditVec C_(nodemap.size(), 0);
	CreditVec Cnorm;
	std::vector<CreditVec> distribs(num_steps);
	std::vector<CreditVec> updates(num_steps);
	std::vector<double> diff_avg(num_steps);
	std::vector<double> stdevs(num_steps);
	double max = 1.0, min = -1.0;

	for (int i=0; i<num_steps; ++i) {
		printf("round %d = ", i+1);

		t1=clock();
		credit_update(C, C_);
		t2=clock();
		printf( "%f seconds\n", utils::elapsed_time(t1, t2) );

		// compute and store the average squared difference between C(t-1,i) and C(t, i)
		// diff_avg[i] = utils::compute_diff_avg(C, C_);

		// normalize C and store distribution convergence for plotting
		// Cnorm = utils::normalize(C);
		// distribs[i] = Cnorm;

		// compute stdev and store for plotting
		// stdevs[i] = utils::compute_stdev(Cnorm);

		// store credit update before overwriting timestep t
		updates[i] = C_;

		C = C_; // C(t+1) becomes C(t) for next iteration
	}

	// output credit value results after the final step
	printf( "\nOutputting Network and Random Walk Data to %s\n", output_file.c_str() );
	write_output( output_file, updates );

	// store data to file for convergence plotting
	utils::save_results(distribs, diff_avg, stdevs);

	// free heap memory
	for (auto& kv: nodemap) {
		delete kv.second;
	}

	return 0 ;
}
