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
int *scounts, *rcounts, *sdisp, *rdisp, ssize, rsize;
ExtNode *snodes, *rnodes;


/*
 * Reads the network from the given file into the global adjacency list unordered map.
 */
void load_network( std::string edge_view_file, std::string partition_file ) {
	GraphSize source, target, degree, max, cur_size;
	int partition;
	Node *srcnode, *tarnode;

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
                //printf( "source %lu index %lu\n", source, srcnode->index() );
			}
		}
	}
	partfile.close();
    //assert( false );
}

void init_message_buffers() {
	Node *node;
	GraphSize partition;

	printf( "Entering init_message buffers\n" );
	
	// inti storage
	// scounts = new unsigned long[numtasks];
	// rcounts = new unsigned long[numtasks];
	// sdisp = new unsigned long[numtasks];
	// rdisp = new unsigned long[numtasks];
	scounts=(int*)malloc(sizeof(int)*numtasks);
	rcounts=(int*)malloc(sizeof(int)*numtasks);
	sdisp=(int*)malloc(sizeof(int)*numtasks);
	rdisp=(int*)malloc(sizeof(int)*numtasks);

	// count sends
	for ( int i=0; i<numtasks; ++i ){
		scounts[i] = 0;
	}
	for ( auto& id : partvec ) {
		node = nodevec[id];
		for ( auto& edge_node : *(node->getEdges()) ) {
			partition = edge_node->partition();
			if ( partition != taskid ) {
				scounts[partition]++;
			}
		}
	}
	for ( int i=0; i<numtasks; ++i ) {
		printf( "parition %d scounts[%d] = %d\n", taskid, i, scounts[i] ); 
	}

	// exchange send count info
    printf( "Alltoall\n" );
	MPI_Alltoall( scounts, 1, MPI_INT,
				  rcounts, 1, MPI_INT,
				  MPI_COMM_WORLD );
	printf( "Alltoall finished\n" );

	for ( int i=0; i<numtasks; ++i ) {
		printf( "parition %d rcounts[%d] = %d\n", taskid, i, rcounts[i] ); 
	}

	// calculate displacements and buffer sizes
	sdisp[0]=0;
	for( int i=1; i<numtasks; ++i ) {
		sdisp[i] = scounts[i-1] + sdisp[i-1];
	}
	rdisp[0]=0;
	for( int i=1; i<numtasks; ++i ) {
		rdisp[i] = rcounts[i-1] + rdisp[i-1];
	}
	ssize = 0;
	rsize = 0;
	for( int i=0; i<numtasks; ++i ) {
		ssize += scounts[i];
		rsize += rcounts[i];
	}
	for ( int i=0; i<numtasks; ++i ) {
		printf( "parition %d sdisp[%d] = %d rdisp[%d] = %d\n", taskid, i, sdisp[i], i, rdisp[i] ); 
	}
	printf( "parition %d computed sizes ssize = %d rrsize = %d\n", taskid, ssize, rsize );

	// initialize send/receive buffers
	snodes = (ExtNode*)malloc(sizeof(ExtNode)*ssize); //new ExtNode[ssize];
	rnodes = (ExtNode*)malloc(sizeof(ExtNode)*rsize); //new ExtNode[rsize];
	for ( GraphSize i=0; i<ssize; ++i ) {
		snodes[i] = ExtNode();
	}
	for ( GraphSize i=0; i<rsize; ++i ) {
		rnodes[i] = ExtNode();
	}
    printf( "Finished init\n" );
}

void communicate_credit_updates() {
	Node *node;
	// ExtNode *extnode;
	GraphSize id;
	int partition;
	//std::vector<int> disp_counter( sdisp, sdisp + sizeof(sdisp) / sizeof(sdisp[0]) );
	int *disp_counter = int[numtasks];
	memcopy( disp_counter, sdisp, sizeof(int)*numtasks );
	for ( int i=0; i<numtasks; ++i ) {
		printf( "partition %d disp_counter[%d] %d\n", taskid, i, disp_counter[i] );
		// disp_counter.push_back()
	}
	// for ( auto& c : disp_counter ) {
	// 	printf( "partition %d c %d\n", taskid, c );
	// }
    
    printf( "partition %d Entering Comm Credit Updates\n", taskid );

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
	printf( "Alltoallv\n" );
	MPI_Alltoallv( snodes, scounts, sdisp, ExtNode_type,
				   rnodes, rcounts, rdisp, ExtNode_type,
				   MPI_COMM_WORLD );
	printf( "Alltoallv finished\n" );

	// update local Node credit values
	for ( int i=0; i<rsize; ++i ) {
		id = rnodes[i].id;
		node = nodevec[id];
		node->setCredit( rnodes[i].credit );
		if ( id == -1 ) {
			printf( "ERROR: -1 id in received.\n" );
			// node = nodevec[id];
			// node->setCredit( rnodes[i].credit );
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
	#pragma omp parallel for private( sum, node, id ) shared( C, C_ )
	for ( GraphSize i = 0; i < partvec.size(); ++i ) {
		node = nodevec[partvec[i]];
		id = node->id();
		sum = 0;
		for ( auto& tarnode: *(node->getEdges()) ) {
			// if ( tarnode->partition() == taskid ) {
			// 	sum += C[tarnode->index()];
			// 	node->setCredit( C[node->index()] );
			// } else {
				sum += tarnode->credit() / tarnode->degree();
			// }
		}
        // printf( "sum = %f node->index() = %lu\n", sum, node->index() );
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
	// char hostname[MPI_MAX_PROCESSOR_NAME];
	// MPI_Status status;
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
	// load nodes
	if ( is_master ) {
		printf( "Reading input files %s %s:\n", 
				partition_file.c_str(), edge_view_file.c_str() );
	}
	start = omp_get_wtime();
	load_network( edge_view_file, partition_file  );
	end = omp_get_wtime();
	printf( "time to read input files, partition %d = %lf seconds\n", taskid, end - start );


	/* INITIALIZE MESSAGE PASSING INFRASTRUCTURE */
	// define a custom datatype for external node credit info
	MPI_Datatype ExtNode_type;
    MPI_Datatype types[2] = { MPI_UNSIGNED_LONG, MPI_DOUBLE };
    int blocklen[2] = { 1, 1 };
    MPI_Aint extent, lower_bound;
    MPI_Type_get_extent( MPI_UNSIGNED_LONG, &lower_bound, &extent );
    MPI_Aint offsets[2] = { 0, 1*extent };
    MPI_Type_create_struct(2, blockcounts, offsets, types, &ExtNode_type);
    MPI_Type_commit(&ExtNode_type);

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
		end = omp_get_wtime();
		printf( "--- time for round %d, partition %d = %f seconds\n", i+1, taskid, end - start );

		// send/recieve credit updates
		communicate_credit_updates();

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
	// delete scounts;
	// delete rcounts;
	// delete sdisp;
	// delete rdisp;
	// delete snodes;
	// delete rnodes;
	free( scounts );
	free( rcounts );
	free( sdisp );
	free( rdisp );
	free( snodes );
	free( rnodes );
	for ( auto& node: nodevec ) {
		delete node;
	}
	MPI_Finalize();
	
	return 0 ;
}

	
