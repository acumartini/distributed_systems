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


// global network storage
std::vector<GraphSize> partvec; // stores id's of local nodes
NodeVec nodevec;

// distributed network variables
int  numtasks, taskid;
bool is_master;
int num_rounds;

// message passing variables
MPI_Datatype ext_node_type;
int *scounts, *rcounts, *sdisp, *rdisp, ssize, rsize;
ExtNode *snodes, *rnodes;


/*
 * Reads the network from the given file into the global adjacency list unordered map.
 */
void load_network( std::string edge_view_file, std::string partition_file ) {
	GraphSize source, target, degree, max, cur_size;
	int partition;
	Node *node, *srcnode, *tarnode;

	// load edgeview file and iterate over each line
	std::ifstream edgefile( edge_view_file );
	if ( edgefile.is_open() ) {
		while ( edgefile >> source >> target ) { 
			// check for resize
			if ( source >= nodevec.size() || target >= nodevec.size() ) {
				max = std::max( source, target );
				cur_size = nodevec.size();
				nodevec.resize( max + 1 );
				while ( cur_size <= max ) {
					nodevec[cur_size++] = new Node();
				}
			}

			// get source node
			srcnode = nodevec[source];
			srcnode->setId( source );

			// get target node
			tarnode = nodevec[target];
			tarnode->setId( target );

			// add new undirected edge
			srcnode->addEdge( tarnode );
			tarnode->addEdge( srcnode );
		}
	}
	edgefile.close( );

	// load partition file and iterate over each line
	std::ifstream partfile( partition_file );
	if ( partfile.is_open() ) {
		GraphSize cur_index = 0;
		while ( partfile >> source >> degree >> partition ) {
			srcnode = nodevec[source];
			srcnode->setDegree( degree );
			srcnode->setPartition( partition );

			// record id and index for local nodes
			if ( taskid == partition ) {
				partvec.push_back( source );
				srcnode->setIndex( cur_index++ );
			}
		}
	}
	partfile.close();
}

void init_message_buffers() {
	Node *node;
	GraphSize local_max, partition;
	
	scounts = new int[numtasks];
	for ( int i=0; i<numtasks; ++i ){
		scounts[i] = 0;
	}

	for ( auto& id : partvec ) {
		node = nodevec[id];
		for ( auto& edge_node : node->getEdges() ) {
			partition = edge_node.partition();
			if ( partition != taskid ) {
				scounts[partition]++;
			}
		}
	}

	// exchange send count info
	MPI_Alltoall( &scounts, 1, MPI_INT,
				  &rcounts, 1, MPI_INT,
				  MPI_COMM_WORLD );

	// calculate displacements and buffer sizes
	sdisp[0]=0;
	for( int i=1; i<numtasks; ++i ) {
		sdisp[i] = scounts[i-1] + sdisp[i-1];
	}
	rdisp[0]=0;
	for( int i=1; i<numtasks; ++i ) {
		rdisp[i] = rcounts[i-1] + rdisp[i-1];
	}
	ssize=0;
	rsize=0;
	for( int i=1; i<numtasks; ++i ) {
		ssize += scounts[i];
		rsize += rcounts[i];
	}

	// initialize send/receive buffers
	snodes = new ExtNode[ssize];
	rnodes = new ExtNode[rsize];
	for ( int i=0; i<ssize; ++i ) {
		snodes[i] = new ExtNode();
	}
	for ( int i=0; i<rsize; ++i ) {
		rnodes[i] = new ExtNode();
	}
}

void communicate_credit_updates() {
	Node *node;
	ExtNode *extnode;
	GraphSize id;
	int partition;
	std::vector<int> disp_counter( numtasks, 0 );

	// populate sending nodes with credit updates
	for ( auto& id : partvec ) {
		node = nodevec[id];
		for ( auto& tarnode : node->getEdges() ) {
			partition = tarnode->partition() ;
			if ( taskid != partition ) {
				extnode = snodes[disp_counter[partition]];
				extnode->id = node->id();
				extnode->credit = node->credit();
				disp_counter[partition]++;
			}
		}
	}

	// communicate individual to all other nodes
	MPI_Alltoallv( snodes, scounts, sdisp, ext_node_type,
				   rnodes, rcounts, rdisp, ext_node_type,
				   MPI_COMM_WORLD );

	// update local Node credit values
	for ( int i=0; i<rsize; ++i ) {
		id = rnodes[i]->id;
		if ( id != -1 ) {
			node = nodevec[id];
			node->setCredit( rnodes[i]->credit );
		}
	}
}

/*
 * Computes a credit update for the next time step given the current credit values for 
 * each node in the network.  Uses paralellism by mapping updates to available cores
 * through the OpenMP framework.
 * 
 * @params: C - stores the credit values for each node at time step t+1
 */
void credit_update ( CreditVec &C ) {
	double sum;
	GraphSize id;
	Node *node;

	// compute credit for the next time step
	#pragma omp parallel for private( sum, node, id ) shared( C )
	for ( GraphSize i = 0; i < nodevec.size(); ++i ) {
		node = nodevec[i];
		id = node->id();
		if ( id != -1 && taskid == node->partition() ) {
			sum = 0;
			for ( auto& tarnode: *(node->getEdges()) ) {
				sum += tarnode->credit() / tarnode->degree();
			}
			C[node->index()] = sum;
		}
	}
}

/*
 * Writes the network and random walk information to the given file.
 */
void write_output ( std::vector<CreditVec> updates ) {
	FILE * pfile = fopen ( std::stoi( taskid ) + ".out", "w" );
	GraphSize id;
	Node *node;
	
	// sort nodevec by id
   	std::sort( partvec.begin(), partvec.end(), nodecomp() );

	for ( auto& id : partvec ) {
		node = nodevec[id];
		fprintf( pfile, "%lu\t%lu", id, node->degree() );
		// output update values for node n
		for ( int i = 0; i < updates.size(); ++i ) {
			fprintf( pfile, "\t%.6lf", updates[i][node->index()] );
		}
		fprintf( pfile, "\n" );
	}

	fclose( pfile );
}


int main (int argc, char *argv[]) {
	// initialize/populate mpi specific vars local to each node
	char hostname[MPI_MAX_PROCESSOR_NAME];
	MPI_Status status;
	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &numtasks );
	MPI_Comm_rank( MPI_COMM_WORLD, &taskid );
	is_master = ( tasid == MASTER ) ? true : false;

    // handle cmd args
    std::string partition_file;
	std::string edge_view_file;

	if ( argc != 3 ) {
		if ( is_master ) {
			printf( "Usage: mpiexec -np <num_partitions> ./rws_mpi "
					"<nodes_to_partition> <edge_view> <num_rounds>\n");
		}
		MPI_Finalize();
		exit( 0 );
	} else {
		partition_file = argv[1];
		edge_view_file = argv[2];
		num_rounds = atoi( argv[3] );
	}

	// check for valid input files
	if ( !utils::fexists( partition_file ) ) { 
		if ( is_master ) {
			printf( "Error: Partition file does not exist.\n" );
		}
		MPI_Finalize();
		exit( 0 );
	}
	if ( !utils::fexists( edge_view_file ) ) { 
		if ( is_master ) {
			printf( "Error: Edge View file does not exist.\n" );
		}
		MPI_Finalize();
		exit( 0 );
	}


	/* NETWORK INITIALIZATION */
	// load nodes
	if ( is_master ) {
		printf( "Reading input files %s, %s:\n", 
				partition_file.c_str(), edge_view_file.c_str() );
	}
	start = omp_get_wtime();
	load_network( edge_view_file, partition_file  );
	end = omp_get_wtime();
	printf( "time to read input files, partition %d = %lf seconds\n", taskid, end - start );


	/* INITIALIZE MESSAGE PASSING INFRASTRUCTURE */
	// required data types to define a custom datatype for MPI
    MPI_Datatype ext_node_type, oldtypes[2];
    int blockcounts[2];
    MPI_Aint offsets[2], extent, lower_bound;

    // setup ext_node id
    offsets[0] = 0;
    oldtypes[0] = MPI_UNSIGNED_LONG;
    blockcounts[0] = 1;

    // setup ext_node credit
    MPI_Type_get_extent(MPI_UNSIGNED_LONG, &lower_bound, &extent);
    offsets[1] = 1*extent;
    oldtypes[1] = MPI_DOUBLE;
    blockcounts[1] = 1;

    // define structured data type and commit it into the MPI environment
    MPI_Type_create_struct(2, blockcounts, offsets, oldtypes, &ext_node_type);
    MPI_Type_commit(&ext_node_type);

    // initialize all-to-all buffers
	init_message_buffers();


	/* PERFORM RANDOM WALKS */
	MPI_Barrier( MPI_COMM_WORLD );
	if ( is_master ) { 
		printf("\nComputing the Credit Values for %d Rounds:\n", num_rounds);
	}

	CreditVec C( partvec.size(), 0 );
	std::vector<CreditVec> updates( num_rounds );

	for (int i=0; i<num_rounds; ++i) {
		// send/recieve credit updates
		if ( i != 0 ) { // skip first round
			communicate_credit_updates();
		}

		// compute credit update
		start = omp_get_wtime();
		credit_update( C );

		// store credit update before overwriting timestep t
		updates[i] = C;
		end = omp_get_wtime();
		printf( "--- time for round %d, partition %d = %f seconds\n", i+1, taskid, end - start );

		// wait for all processes to finish
		MPI_Barrier( MPI_COMM_WORLD );
	}


	/* MODEL STORAGE */
	if ( is_master ) {
		printf( "\nOutputting network and random walk data for each partition.\n" );
	}
	write_output( updates );


	/* FINALIZE */
	// free heap memory
	for ( auto& node: nodevec ) {
		delete node;
	}

	MPI_Finalize();
	return 0 ;
}

	