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
MPI_Datatype ExtNode_type;
int *counts, *disp, size;
ExtNode *snodes, *rnodes;


/*
 * Reads the network from the given file into the global adjacency list unordered map.
 */
void load_network( std::string edge_view_file, std::string partition_file ) {
	GraphSize source, target, degree, max, cur_size;
	int partition;
	Node *srcnode, *tarnode;

	// load edge view file and iterate over each line
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
	GraphSize partition;

	// init storage
	counts = (int*) malloc( sizeof(int)*numtasks );
	disp = (int*) malloc( sizeof(int)*numtasks );

	// count external dependencies
	for ( int i=0; i<numtasks; ++i ){
		counts[i] = 0;
	}
	for ( auto& id : partvec ) {
		node = nodevec[id];
		for ( auto& edge_node : *(node->getEdges()) ) {
			partition = edge_node->partition();
			if ( partition != taskid ) {
				counts[partition]++;
			}
		}
	}

	// compute send/receive displacements
	disp[0]=0;
	for( int i=1; i<numtasks; ++i ) {
		disp[i] = counts[i-1] + disp[i-1];
	}
	size = 0;
	for( int i=0; i<numtasks; ++i ) {
		size += counts[i];
	}

	// initialize send/receive buffers
	snodes = (ExtNode*)malloc(sizeof(ExtNode)*size);
	rnodes = (ExtNode*)malloc(sizeof(ExtNode)*size);
	for ( GraphSize i=0; i<size; ++i ) {
		snodes[i] = ExtNode();
	}
	for ( GraphSize i=0; i<size; ++i ) {
		rnodes[i] = ExtNode();
	}
}

void communicate_credit_updates() {
	Node *node;
	GraphSize id;
	int partition;

    std::vector<int> disp_counter;
    for ( int i=0; i<numtasks; ++i ) {
		disp_counter.push_back( disp[i] );
	}
    
	// populate sending nodes with credit updates
	for ( auto& id : partvec ) {
		node = nodevec[id];
		for ( auto& tarnode : *(node->getEdges()) ) {
			partition = tarnode->partition() ;
			if ( taskid != partition ) {
				snodes[disp_counter[partition]].id = node->id();
				snodes[disp_counter[partition]].credit = node->credit();
				disp_counter[partition]++;
			}
		}
    }

	// communicate individual to all other nodes
	MPI_Alltoallv( snodes, counts, disp, ExtNode_type,
				   rnodes, counts, disp, ExtNode_type,
				   MPI_COMM_WORLD );

	// update local Node credit values
	for ( int i=0; i<size; ++i ) {
		id = rnodes[i].id;
		node = nodevec[id];
		node->setCredit( rnodes[i].credit );
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
	for ( GraphSize i = 0; i < partvec.size(); ++i ) {
		node = nodevec[partvec[i]];
		id = node->id();
		sum = 0;
		for ( auto& tarnode: *(node->getEdges()) ) {
			sum += tarnode->credit() / tarnode->degree();
		}
		C[node->index()] = sum;
	}

	// update credit for nodes in this partition
	#pragma omp parallel for private( node )
	for ( GraphSize i = 0; i < partvec.size(); ++i ) {
		node = nodevec[partvec[i]];
		node->setCredit( C[node->index()] );
	}
}

/*
 * Writes the network and random walk information to the given file.
 */
void write_output ( std::vector<CreditVec> updates ) {
	FILE *pfile = fopen ( (std::to_string( taskid ) + ".out").c_str(), "w" );
	Node *node;
	
	// sort nodevec by id
   	std::sort( partvec.begin(), partvec.end() );

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
	double start, end;

	// initialize/populate mpi specific vars local to each node
	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &numtasks );
	MPI_Comm_rank( MPI_COMM_WORLD, &taskid );
	is_master = ( taskid == MASTER ) ? true : false;

    // handle cmd args
    std::string partition_file;
	std::string edge_view_file;

	if ( argc != 4 ) {
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
	// load network
	if ( is_master ) {
		printf( "Reading input files:\n\tnodes to partition - %s\n\tedge view - %s\n", 
				partition_file.c_str(), edge_view_file.c_str() );
	}
	start = omp_get_wtime();
	load_network( edge_view_file, partition_file  );
	end = omp_get_wtime();
	printf( "time to read input files, partition %d = %lf seconds\n", taskid, end - start );


	/* INITIALIZE MESSAGE PASSING INFRASTRUCTURE */
	// define a custom datatype for sending external node credit structs
    MPI_Datatype types[2] = { MPI_LONG, MPI_DOUBLE };
    int blocklen[2] = { 1, 1 };
    MPI_Aint extent, lower_bound;
    MPI_Type_get_extent( MPI_LONG, &lower_bound, &extent );
    MPI_Aint offsets[2] = { 0, 1*extent };
    MPI_Type_create_struct( 2, blocklen, offsets, types, &ExtNode_type );
    MPI_Type_commit( &ExtNode_type );

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
		// compute credit update
		start = omp_get_wtime();
		credit_update( C );

		// store credit update before overwriting timestep t
		updates[i] = C;

		// send/recieve credit updates
		communicate_credit_updates();

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
	free( counts );
	free( disp );
	free( snodes );
	free( rnodes );
	for ( auto& node: nodevec ) {
		delete node;
	}
	MPI_Finalize();
	
	return 0 ;
}

	
