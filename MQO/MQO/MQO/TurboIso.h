#pragma once

#include"TraversalAlgorithm.h"
#include"ComboLinkedLists.h"
#include<vector>
#include<map>
#include <chrono>


using namespace std;



class TurboIso {
private:

	class NECNode {
	public:
		int vertexId;
		std::vector<int> childList;
		int parent;
		int id;
		int label;
		~NECNode(){
			childList = std::vector<int>();
		}
	};

private:
    std::chrono::steady_clock::time_point startTime;
	AdjacenceListsGRAPH * dataGraph;
	std::vector<int> * numberOfEmbeddings;
	std::vector<std::vector<CacheEmbeddingNode *>> * resultCaches;

public:
	int recursiveCallsNo;

private:

	bool needReturn;
	int CRRecursiveCallsNo;

private:

	AdjacenceListsGRAPH * queryGraph;
	bool needFormatCache;

private:

	int * embedding; /// mapping M: V(q) -> V(g), the arrayId is the query vertex id
	std::map<int, int> inverseEmbedding; ///inverse mapping W: V(g) -> V(q)

	//@TurboIso
	int startQueryVertex;
	AdjacenceListsGRAPH::Vertex * startQueryVertexAddress;

	/*
	* The key idea of the CR(candidate subregions) is "locality".
	* We have to use set<int> to storage each tuple in the CR, otherwise there can be some duplications causing error
	*/
	map<std::pair<int, int>, set<int>> CR;
	std::vector<NECNode> necTree;
	std::vector<int> queryMatchingSuquence; //vertexId, parentId;

public:

	TurboIso(AdjacenceListsGRAPH * pDataGraph, std::vector<int> * pNumberOfEmbeddings, std::vector<std::vector<CacheEmbeddingNode *>> * pResultCaches);
	~TurboIso();
	double totalTime = 0;
	std::vector<std::vector<int>*> filterCandidateList;
	void setParameters(AdjacenceListsGRAPH * pQueryGraph, bool pNeedToSaveCache);

	void execute();

private:

	/* TurboIsoBoosted query processing algorithm */
	int chooseStartVertex();
	void rewriteToNecTree();

	/*
	* The key idea for exploring the CR is locality, which is to save the subregion into the CR table and then do further verification
	* @param parentMappedNecTree is the data vertex mapped to u's parent in nec tree
	* @param subRegionCandidates is the set of candidates of u within this subRegion
	*/
	bool exploreCR(int u, int parentMappedNecTree, vector<int> & subRegionCandidates);


	void computeMatchingOrder();
	void devideNoTreeEdges(int u, int numberOfNoTreeEdges, float * queryVertexScore);
	void getOrderByBFSScore(int u, float * queryVertexScore, vector<int> & nextVertex); // after get the scroe for each query vertex, get the matching order by BFS

																						/* TurboIsoBoosted subgraph isomorphism search */
	void updateState(int u, int v);
	void subgraphSearch();
	bool refineCandidates(AdjacenceListsGRAPH::Vertex & u, const int & v);
	bool degreeFilter(int u, int v);
	bool isJoinable(int u, int v);
	NECNode * nextQueryVertex();
	void restoreState(int u, int v);
	

	//@common
	/* Common Utility Function  */
	void showEmbedding();

};
