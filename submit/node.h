/*
 * node.h
 *
 *  Created on: Apr 12, 2014
 *      Author: martini
 */

#ifndef NODE_H_
#define NODE_H_

#include <fstream>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <iterator>
#include <vector>
#include <cmath>
#include <time.h>


typedef unsigned long GraphSize;

class Node {
typedef std::unordered_set<Node*> EdgeSet;

public:
	Node ( const GraphSize& id ): node_id( id ) {}
	~Node () {}

	const EdgeSet& getEdges() const { return edge_set; }
	GraphSize edgeCount () { return edge_set.size(); }

	const GraphSize& id () const { return node_id; }
	const GraphSize& index () const { return node_index; }

	void setIndex ( const GraphSize& index ) { node_index = index; }
  	void addEdge ( Node *node ) { edge_set.insert( node ); }

  	void printEdges() {
		printf( "node %ld with edgeCount %ld\n", node_id, edgeCount() );
		for ( auto& node : edge_set ){
			printf ( "\t%ld\n", node->id() );
		}
	}

private:
	GraphSize node_id;
	GraphSize node_index;
	EdgeSet edge_set;
};

// Node comparator for sorting
struct nodecomp
{
     bool operator() ( Node* n1, Node* n2 )
    {
        return ( n1->id() < n2->id() );
    }
};

typedef std::vector<Node*> NodeVec;
typedef std::unordered_map<GraphSize, Node*> NodeMap;
typedef std::pair<GraphSize, GraphSize> NodePair;
typedef std::vector<double> CreditVec;

#endif /* NODE_H_ */
