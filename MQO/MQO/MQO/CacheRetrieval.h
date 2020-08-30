#pragma once
#include"AdjacenceListsGraph.h"
#include"CacheEmbeddingNode.h"
#include"JointNodeOrderNode.h"
#include"JointGraph.h"
#include<vector>
#include<map>

class CacheRetrieval {
	
private:
	AdjacenceListsGRAPH * dataGraph;
	std::vector<std::vector<CacheEmbeddingNode *>> * resultCaches;

private:

	AdjacenceListsGRAPH * queryGraph;
	AdjacenceListsGRAPH * parentGraph;
	JointNodeOrderNode * jointNodeOrderNode;
	JointGraphNode * jointGraphNode;
	int * embedding;
	std::map<int, int> * inverseEmbedding;
	std::vector<std::vector<CacheEmbeddingNode *>> * candidates;

private:
	
	CacheEmbeddingNode ** comboVertexMapping;

public:

	CacheRetrieval(AdjacenceListsGRAPH * pDataGraph, std::vector<std::vector<CacheEmbeddingNode *>> * pResultCaches);

	void retrieveEmbeddingCandidate(AdjacenceListsGRAPH * pQueryGraph, AdjacenceListsGRAPH * pParentGraph, JointGraphNode * pJointGraphNode, JointNodeOrderNode * pJointNodeOrderNode, int * pEmbedding, std::map<int, int> * pInverseEmbedding, std::vector<std::vector<CacheEmbeddingNode *>> * pCandidates);

private:

	void recursiveEnumLinkedLists(int comboVertexOrder);

	bool isJoinable(CacheEmbeddingNode * embeddingNode, ComboNODE * comboNode);

	bool degreeFilter(int u, int v);

};