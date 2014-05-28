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
	Node ( const GraphSize& id ): 
		node_id( id ),
		edge_set( new EdgeSet() ),
		cur_credit( 1 ) {}
	~Node () { delete edge_set; }

	const GraphSize& id () const { return node_id; }
	const GraphSize& partition () const { return part_id; }
	const EdgeSet* getEdges() const { return edge_set; }
	GraphSize degree () const { return node_degree; }
	double credit () const { return cur_credit; }
	GraphSize index() const { return local_index; }

	void setId ( const GraphSize& id ) { node_id = id; }
	void setPartition ( const GraphSize& partition ) { part_id = partition; }
	void setDegree ( const GraphSize& degree ) { node_degree = degree; }
  	void addEdge ( Node* node ) { edge_set->push_back( node ); }
  	void setCredit ( const double& credit ) { cur_credit = credit; }
  	void setIndex ( const GraphSize& index ) { local_index = index; }

private:
	GraphSize node_id;
	GraphSize part_id;
	GraphSize node_degree;
	EdgeSet *edge_set;
	double cur_credit;
	GraphSize local_index;
};

// Node comparator for sorting
struct nodecomp {
	bool operator() ( Node* n1, Node* n2 ) {
        return ( n1->id() < n2->id() );
    }
};

typedef std::vector<Node*> NodeVec;
typedef std::vector<double> CreditVec;

// External Node communication structure
struct ExtNode {
	ExtNode (): id( -1 ), credit( -1 ) {}
	ExtNode ( const GraphSize& node_id, const double& cur_credit ):
		id( node_id ),
		credit( cur_credit ) {}
    
    GraphSize id;
    double credit;
};

#endif /* NODE_H_ */
