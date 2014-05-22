/* Adam Martini
 * CIS 630 Distributed Systems
 * Project 1
 * rws.cpp
 * 4.20.14
 */

#include <string>
#include <algorithm>
#include <stdio.h>
#include <assert.h>

#include "omp.h"
#include "utils.h"
#include "node.h"


// global network storage
NodeVec nodevec;


/*
 * Reads the network from the given file into the global adjacency list unordered map.
 */
void load_network(std::string filename) {
	GraphSize source, target, max, cur_size;
	NodePair nodes;
	Node *node, *srcnode, *tarnode;

	// load file and iterate through each line of input
	std::ifstream infile(filename);
	if ( infile.is_open() ) {
		// while ( getline(infile, line, '\n') ) {
		while ( infile >> source >> target ) { 
			// verify valid edge
			if (source != target) {
				// check for resize
				max = std::max( source, target );
				if ( max >= nodevec.size() ) {
					cur_size = nodevec.size();
					nodevec.resize( max + 1 );
					while ( cur_size <= max ) {
						nodevec[cur_size++] = new Node();
					}
				}

				// get source node
				srcnode = nodevec[source];
				if ( srcnode->id() == -1 ) {
					srcnode->setId( source );
				}

				// get target node
				tarnode = nodevec[target];
				if ( tarnode->id() == -1 ) {
					tarnode->setId( target );
				}

				srcnode->addEdge( tarnode );
				tarnode->addEdge( srcnode );
			}
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
	GraphSize id;
	Node *node;

	// compute credit for the next time step
	#pragma omp parallel for private( sum, node, id ) shared( C, C_ )
	for ( GraphSize i = 0; i < nodevec.size(); ++i ) {
		node = nodevec[i];
		id = node->id();
		if ( id != -1 ) {
			sum = 0;
			for ( auto& tarnode: *(node->getEdges()) ) {
				// i = tarnode->id();
				sum += C[tarnode->id()] / tarnode->edgeCount();
			}
			// i = node->index();
			C_[id] = sum;
		}
	}
}

/*
 * Writes the network and random walk information to the given file.
 */
void write_output ( std::string filename, std::vector<CreditVec> updates ) {
	FILE * pfile = fopen ( filename.c_str(), "w" );
	GraphSize id;
	
	// sort nodevec by id
   	std::sort( nodevec.begin(), nodevec.end(), nodecomp() );

	for ( auto& node : nodevec ) {
		id = node->id();
		if ( id != -1 ) {
			fprintf( pfile, "%lu\t%lu", id, node->edgeCount() );
			// output update values for node n
			for ( int i = 0; i < updates.size(); ++i ) {
				fprintf( pfile, "\t%.6lf", updates[i][id] );
			}
			fprintf( pfile, "\n" );
		}
	}

	fclose( pfile );
}


int main ( int argc , char** argv ) {
	double start, end;

	// get cmdline args
	std::string input_file = argv[1];
	std::string output_file = argv[2];
	int num_steps = atoi( argv[3] );

	// check for valid input file
	if ( !utils::fexists( input_file ) ) { 
		printf( "Error: Input file does not exist.\n" );
		return 1;
	}

	// initialize adjacency list vector hash
	printf( "Loading network edges from %s\n", input_file.c_str() );
	start = omp_get_wtime();
	load_network( input_file );
	end = omp_get_wtime();
	printf( "Time to read input file = %lf seconds\n", end - start );
	
	// compute the normalized credit after numSteps
	printf("\nComputing the Credit Values for %d Rounds:\n", num_steps);
	CreditVec C( nodevec.size(), 1 ); // initialize credit at t=0 to 1 for each node
	CreditVec C_( nodevec.size(), 0 );
	std::vector<CreditVec> updates( num_steps );

	for (int i=0; i<num_steps; ++i) {
		printf("round %d = ", i+1);

		start = omp_get_wtime();
		credit_update(C, C_);
		end = omp_get_wtime();
		printf( "%f seconds\n", end - start );

		// store credit update before overwriting timestep t
		updates[i] = C_;

		C = C_; // C(t+1) becomes C(t) for next iteration
	}

	// output credit value results after the final step
	printf( "\nOutputting Network and Random Walk Data to %s\n", output_file.c_str() );
	write_output( output_file, updates );

	// free heap memory
	for ( auto& node: nodevec ) {
		delete node;
	}

	return 0 ;
}
