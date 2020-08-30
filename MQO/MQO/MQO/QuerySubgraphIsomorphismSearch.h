#pragma once

#include"AdjacenceListsGraph.h"
#include<vector>
#include<map>
#include <chrono>


using namespace std;

/*
* We just use the most simple Ullmann algorithm to search subgraph isomorphism
* This is because, the query graphs are small graphs. using advanced graph isomorphism will be even slower then the simple one
* To use this, just make sure both the data graph and query graph have the (label->vertexlist index available
*/

class QuerySubgraphIsomorphismSearch {
private:
    std::chrono::steady_clock::time_point startTime;

	AdjacenceListsGRAPH * dataGraph;
	AdjacenceListsGRAPH * queryGraph;
	vector<vector<int>> * storageMappings;

private:

	std::map<int, vector<int>* > candidates;
	int * embedding;
	std::map<int, int> inverseEmbedding; ///inverse mapping W: V(g) -> V(q)

private:

	int enoughtEmbeddingsNumber;
	bool enoughtEmbeddingsFound;

public:

	void subgraphIsomorphismSearch(AdjacenceListsGRAPH * pDataGraph, AdjacenceListsGRAPH * pQueryGraph, vector<vector<int>> * storageMappings);

private:

	void filterCandidates();

	void isomorphismSearch();

	AdjacenceListsGRAPH::Vertex nextQueryVertex();

	void updateState(int u, int v);

	void restoreState(int u, int v);

	bool isJoinable(int u, int v);

	bool refineCandidates(AdjacenceListsGRAPH::Vertex & u, const int & v);

	bool degreeFilter(int u, int v);

	void showEmbedding();


};
