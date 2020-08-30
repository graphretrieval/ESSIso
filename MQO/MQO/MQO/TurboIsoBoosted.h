#pragma once


#include<vector>
#include<stack>
#include<queue>
#include<string>
#include<map>
#include<set>

#include"ComboLinkedLists.h"
#include"AdjacenceListsGraph.h"
#include"TraversalAlgorithm.h"


using namespace std;



class TurboIsoBoosted {

private:
	/*
	* As we don't consider the neighbourhood equivalent class for the query graphs here.
	* Therefore, the NECNode is only a common node in the BFS tree.
	*/
	class NECNode {
	public:
		int vertexId;
		vector<int> childList;
		int parent;
		int id;
		int label;
	};

private:

	AdjacenceListsGRAPH * hyperGraph;
	std::vector<int> * numberOfEmbeddings;
	std::vector<std::vector<CacheEmbeddingNode *>> * resultCaches;

private:

	bool needReturn;
	int mappedQueryVertexSize;
	int recursiveCallsNo;
	int CRRecursiveCallsNo;

private:

	AdjacenceListsGRAPH * queryGraph;
	bool needFormatCache;

private:

	int * embedding; /// mapping M: V(q) -> V(g), the arrayId is the query vertex id
	std::map<int, std::stack<int>> inverseEmbedding; ///inverse mapping W: V(g) -> queryList{u1,u2...}
	std::map<int, vector<int>* > scRootCandidates;


private:

	//@TurboIso
	int startQueryVertex;
	AdjacenceListsGRAPH::Vertex * startQueryVertexAddress;

	/*
	 * The key idea of the CR(candidate subregions) is "locality".
	 * We have to use set<int> to storage each tuple in the CR, otherwise there can be some duplications causing error
	 */
	map<std::pair<int,int>, set<int>> CR;
	std::vector<NECNode> necTree;
	std::vector<int> queryMatchingSuquence; //vertexId, parentId; 

	/*
	 * For turboIso, we only build the DRT table for the starting query vertex.
	 * the d_table contains all the DRT units built over the candidates of this query vertex
	 * the superStartingandidates are the DRT units in the d_table which are not qdc-contained by any other DRT unit.
	 */
	std::queue<int> startingQueryVertexCandidates;



public:

	TurboIsoBoosted(AdjacenceListsGRAPH * pHyperGraph, std::vector<int> * pNumberOfEmbeddings, std::vector<std::vector<CacheEmbeddingNode *>> * pResultCaches);

	void setParameters(AdjacenceListsGRAPH * pQueryGraph, bool pNeedToSaveCache);

	void execute();

private:

	/* TurboIsoBoosted query processing algorithm */
	int chooseStartVertex();
	void rewriteToNecTree();
	void filterCandidates();


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
	void updateState(AdjacenceListsGRAPH::Vertex * u, int v);
	void subgraphSearch();
		bool isJoinable(AdjacenceListsGRAPH::Vertex * u, AdjacenceListsGRAPH::Vertex * v);
	void restoreState(AdjacenceListsGRAPH::Vertex * u, int v);
	NECNode * nextQueryVertex();
	
	
	/**
	*	@unique for Boost
	*/
	void generateEquivalentEmbedding();
	void generateQDEquivalentEmbedding();
	void dynamicLoadCandidates(int dataVertexId);



	//@common
	/* Common Utility Function  */
	void showEmbedding();

};
