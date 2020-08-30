#pragma once
#include"AdjacenceListsGraph.h"
#include"JointGraph.h"
#include"PCM_Node.h"
#include"CacheEmbeddingNode.h"
#include"JointNodeOrderNode.h"
#include<vector>
#include<map>
#include<set>
#include <chrono>

using namespace std;

class TurboIsoMQO {


private:
	/*
	 * constructor variables
	 */
	std::chrono::steady_clock::time_point startTime;
	AdjacenceListsGRAPH * dataGraph;
	AdjacenceListsGRAPH * queryGraph;

	std::vector<int> * numberOfEmbeddings;
	std::vector<std::vector<CacheEmbeddingNode *>> * resultCaches;

	std::vector<AdjacenceListsGRAPH> * queryGraphVector;
	std::vector<AdjacenceListsGRAPH> * newlyGeneratedGraphVector;

private:
	/*
	 * execute once variables
	 */
	JointGraph * jointGraph;
	bool needSaveCache;

private:

	bool needReturn;
	int recursiveCallsNo;

	/*
	 * internal variables 
	 */
	int * embedding;
	std::map<int, int> inverseEmbedding; ///inverse mapping W: V(g) -> V(q)

	std::vector<JointNodeOrderNode> jointNodeMatchingOrder;

public:

	TurboIsoMQO(AdjacenceListsGRAPH * pDataGraph, std::vector<std::vector<CacheEmbeddingNode *>> * pResultCaches, std::vector<int> * pNumberOfEmbeddings,
		std::vector<AdjacenceListsGRAPH> * pQueryGraphVector, std::vector<AdjacenceListsGRAPH> * pNewlyGeneratedGraphVector);
	
	~TurboIsoMQO();
	
	void setParameters(AdjacenceListsGRAPH * pQueryGraph, JointGraph * pJointGraph, bool pNeedSaveCache);

	void execute();

private:

	void computeMatchingOrder();

	void resuriveSearch(int matchingOrderIndex);

	void updateState(int u, int v);

	void restoreState(int u);

	bool isJoinable(int u, int v);

	bool degreeFilter(int u, int v);
};