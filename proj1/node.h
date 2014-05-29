/*
 * node.h
 *
 *  Created on: 5-16-2014
 *      Author: martini
 */

#ifndef NODE_H_
#define NODE_H_

#include <vector>
#include <unordered_map>
#include <unordered_set>

typedef unsigned long GraphSize;


class Node {
typedef std::vector<Node*> EdgeSet;

public:
	Node (): node_id( -1 ), edge_set( new EdgeSet() ) {} // default constructor
	Node ( const GraphSize& id ): node_id( id ), edge_set( new EdgeSet() ) {}
	~Node () { delete edge_set; }

	const EdgeSet* getEdges() const { return edge_set; }
	GraphSize edgeCount () { return edge_set->size(); }

	const GraphSize& id () const { return node_id; }

	void setId ( const GraphSize& id ) { node_id = id; }
  	void addEdge ( Node* node ) { edge_set->push_back( node ); }

private:
	GraphSize node_id;
	//GraphSize node_index;
	EdgeSet *edge_set;
};

// Node comparator for sorting
struct nodecomp {
	bool operator() ( Node* n1, Node* n2 ) {
        return ( n1->id() < n2->id() );
    }
};

typedef std::vector<Node*> NodeVec;
typedef std::pair<GraphSize, GraphSize> NodePair;
typedef std::vector<double> CreditVec;

#endif /* NODE_H_ */
