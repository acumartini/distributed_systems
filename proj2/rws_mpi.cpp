/* Adam Martini
 * CIS 630 Distributed Systems
 * Project 2
 * rws_mpi.cpp
 * 5.17.14
 */

#include <string>
#include <algorithm>
#include <stdio.h>
#include <assert.h>

#include "mpi.h"
#include "omp.h"
#include "utils.h"
#include "node.h"

#define  MASTER		0
#define  TAG_0      0
 

// global network storage
NodeVec nodevec;


int main (int argc, char *argv[]) {
    // handle cmd args
	int batch_size;
	if ( argc < 3 ) {
		printf( " Usage: ./rws_mpi <partition_id> <nodes_to_partition> <edge_view> <proc_add[0]> ... <proc_add<N-1>\n");
		exit( 0 );
	} else {
		// get partition id for MASTER process
		batch_size = INT_MIN;
	}

	// initialize/populate mpi specific vars local to each node
	int  numtasks, taskid, len, dest, source;
	char hostname[MPI_MAX_PROCESSOR_NAME];
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
	MPI_Get_processor_name(hostname, &len);


	/***** MASTER TASK ONLY ******/

	// perform data preprocessing based on number of workers and batch_size
	if (taskid == MASTER) {
		printf( "MASTER: Number of MPI tasks is: %d\n",numtasks );

		/* DATA PREPROCESSING */


        /* DATA MARSHALLING */

        
        // send data to processes
  //       long offset = chunksize;
		// for (dest=1; dest<numtasks; dest++) {
		// 	MPI_Send( &chunksize, 1, MPI_LONG, dest, TAG_0, MPI_COMM_WORLD );
		// 	MPI_Send( &numfeats, 1, MPI_LONG, dest, TAG_0, MPI_COMM_WORLD );
		// 	MPI_Send( &numlabels, 1, MPI_LONG, dest, TAG_0, MPI_COMM_WORLD );
		// 	MPI_Send( data.data() + offset * numfeats, chunksize * numfeats, MPI_FLOAT, dest, TAG_0, MPI_COMM_WORLD );
		// 	MPI_Send( labels.data() + offset * numlabels, chunksize * numlabels, MPI_FLOAT, dest, TAG_0, MPI_COMM_WORLD );
		// 	printf( "Sent %ld instances to task %d offset= %ld\n", chunksize, dest, offset );
		// 	offset += chunksize;
		// }


		/* PERFORM RANDOM WALKS */


		/* MODEL STORAGE */

	} 

	/***** NON-MASTER TASKS ONLY *****/

	if (taskid > MASTER) {
		printf ("Hello from task %d on %s!\n", taskid, hostname);

		/* DATA INITIALIZATION */

		// long chunksize, numfeats, numlabels;
		// source = MASTER;
		
		// // recieve data partition
		// MPI_Recv( &chunksize, 1, MPI_LONG, source, MPI_ANY_TAG, MPI_COMM_WORLD, &status );
		// MPI_Recv( &numfeats, 1, MPI_LONG, source, MPI_ANY_TAG, MPI_COMM_WORLD, &status );
		// MPI_Recv( &numlabels, 1, MPI_LONG, source, MPI_ANY_TAG, MPI_COMM_WORLD, &status );
		// printf( "Task %d chunksize = %ld\n", taskid, chunksize );
		// printf( "Task %d numfeats = %ld\n", taskid, numfeats );
		// printf( "Task %d numlabels = %ld\n", taskid, numlabels );

	}

	MPI_Finalize();
}

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
	NodeMap nodemap;
	NodePair nodes;
	Node *node, *srcnode, *tarnode;
	NodeMap::const_iterator got;

	// load file and iterate through each line of input
	std::ifstream infile(filename);
	if (infile) {
		while (getline(infile, line, '\n')) {
		    // convert key to integer and update nodemap with valid input
		    if ( get_nodes(line, nodes) != -1 ) {
				GraphSize source = nodes.first;
				GraphSize target = nodes.second;

				// verify valid edge
				if (source != target) {
					// get source node
					got = nodemap.find( source );
					if ( got == nodemap.end() ) { // create a new source node
						srcnode = new Node( source );
						nodemap[source] = srcnode;
					} else {
						srcnode = got->second;
					}

					// get target node
					got = nodemap.find( target );
					if ( got == nodemap.end() ) { // create a new entry
						tarnode = new Node( target );
						nodemap[target] = tarnode;
					} else {
						tarnode = got->second;
					}

					// update nodes with new (undirected) edge
					srcnode->addEdge( tarnode );
					tarnode->addEdge( srcnode );
				}
			} else {
				std::cout << "Input Error: 2 tokens per line expected." << std::endl;
			}
   		}
   		// populate node vector and add indices
   		nodevec.resize( nodemap.size() );
   		GraphSize i = 0;
   		for ( auto& kv: nodemap ) {
   			node = kv.second;
   			nodevec[i] = node;
   			node->setIndex( i++ );
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
	Node *node;

	// compute credit for the next time step
	#pragma omp parallel for private( sum, i, node ) shared( C, C_ )
	for ( int j = 0; j < nodevec.size(); ++j ) {
		node = nodevec[j];
		sum = 0;
		for ( auto& tarnode: node->getEdges() ) {
			i = tarnode->index();
			sum += C[i] / tarnode->edgeCount();
		}
		i = node->index();
		C_[i] = sum;
	}
}

/*
 * Writes the network and random walk information to the given file.
 */
void write_output ( std::string filename, std::vector<CreditVec> updates ) {
	FILE * pfile = fopen ( filename.c_str(), "w" );
	
	// sort nodevec by id
   	std::sort( nodevec.begin(), nodevec.end(), nodecomp() );

	for ( auto& node : nodevec ) {
		fprintf( pfile, "%lu\t%lu", node->id(), node->edgeCount() );
		// output update values for node n
		for ( int i = 0; i < updates.size(); ++i ) {
			fprintf( pfile, "\t%.6lf", updates[i][node->index()] );
		}
		fprintf( pfile, "\n" );
	}

	fclose( pfile );
}


int main ( int argc , char** argv ) {
	double start, end;

	// get cmdline args
	std::string input_file = argv[1];
	std::string output_file = argv[2];
	int num_steps = atoi( argv[3] );

	// initialize adjacency list vector hash
	printf("Loading network edges from %s\n", input_file.c_str());
	start = omp_get_wtime();
	load_network(input_file);
	end = omp_get_wtime();
	printf( "Time to read input file = %lf seconds\n", end - start );
	
	// compute the normalized credit after numSteps
	printf("\nComputing the Credit Values for %d Rounds:\n", num_steps);
	CreditVec C(nodevec.size(), 1); // initialize credit at t=0 to 1 for each node
	CreditVec C_(nodevec.size(), 0);
	CreditVec Cnorm;
	std::vector<CreditVec> distribs(num_steps);
	std::vector<CreditVec> updates(num_steps);
	std::vector<double> diff_avg(num_steps);
	std::vector<double> stdevs(num_steps);
	double max = 1.0, min = -1.0;

	for (int i=0; i<num_steps; ++i) {
		printf("round %d = ", i+1);

		start = omp_get_wtime();
		credit_update(C, C_);
		end = omp_get_wtime();
		printf( "%f seconds\n", end - start );

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
	for ( auto& node: nodevec ) {
		delete node;
	}

	return 0 ;
}
