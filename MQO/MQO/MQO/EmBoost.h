#pragma once

#include"TraversalAlgorithm.h"
#include"ComboLinkedLists.h"
#include<vector>
#include<map>
#include<faiss/IndexFlat.h>

using namespace std;



class EmBoost {

private:

	AdjacenceListsGRAPH * dataGraph;
	std::vector<int> * numberOfEmbeddings;
	std::vector<std::vector<CacheEmbeddingNode *>> * resultCaches;

public:

	bool needReturn;
	int recursiveCallsNo;
	int CRRecursiveCallsNo;

private:

	AdjacenceListsGRAPH * queryGraph;
	bool needFormatCache;

private:

	int * embedding; /// mapping M: V(q) -> V(g), the arrayId is the query vertex id
	std::map<int, int> inverseEmbedding; ///inverse mapping W: V(g) -> V(q)
	std::map<int, vector<int> > candidates;

	//@TurboIso
	int startQueryVertex;

	/*
	* The key idea of the CR(candidate subregions) is "locality".
	* We have to use set<int> to storage each tuple in the CR, otherwise there can be some duplications causing error
	*/

public:

	EmBoost(AdjacenceListsGRAPH * pDataGraph, std::vector<int> * pNumberOfEmbeddings, std::vector<std::vector<CacheEmbeddingNode *>> * pResultCaches);
	~EmBoost();

	void setParameters(AdjacenceListsGRAPH * pQueryGraph, bool pNeedToSaveCache);

	void execute();
	AdjacenceListsGRAPH::Vertex nextQueryVertex();

private:

	/* TurboIsoBoosted query processing algorithm */
	void filterCandidates();
	// AdjacenceListsGRAPH::Vertex nextQueryVertex();
																			/* TurboIsoBoosted subgraph isomorphism search */
	void updateState(int u, int v);
	void subgraphSearch();
	bool refineCandidates(AdjacenceListsGRAPH::Vertex & u, const int v);
	bool degreeFilter(int u, int v);
	bool isJoinable(int u, int v);
	void restoreState(int u, int v);
	

	//@common
	/* Common Utility Function  */
	void showEmbedding();

};
