#pragma once

#include<vector>
#include"AdjacenceListsGraph.h"
#include"PCM_Node.h"
#include"ExecutionOrderNode.h"


class QueryExecutionOrder {

private: 

	std::vector<PCM_Node> * patternContainmentMap;

	std::vector<ExecutionOrderNode> * executionOrder;

public:

	QueryExecutionOrder(std::vector<PCM_Node> * pPatternContainmentMap, std::vector<ExecutionOrderNode> * pExecutionOrder);

	~QueryExecutionOrder();

	void computeExecutionOrder();

private:

	int getNextPcmNodeId(std::vector<int> & candidates);

	void changeParentsWeight(int graphId);



	/*
	* Given a PCM which is a transitive reduction graph, we need an order for processing each PCM node which points to a graph
	*/
	void DFSTopological(int graphId);

};