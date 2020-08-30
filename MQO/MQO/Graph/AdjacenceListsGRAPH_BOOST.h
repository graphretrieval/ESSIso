#ifndef ADJACENCELISTS_GRAPH_BOOST
#define ADJACENCELISTS_GRAPH_BOOST

#include"AdjacenceListsGraph.h"
#include<fstream>
#include"StringUtility.h"
#include<stack>
#include<map>
#include<vector>
#include<queue>

using namespace std;

class AdjacenceListsGRAPH_BOOST {

public:

	static int isContainmentRelation(AdjacenceListsGRAPH* dataGraph, AdjacenceListsGRAPH* queryGraph, int v, int w, int queryVertexId);
	static int isEquivalentRelation(AdjacenceListsGRAPH* dataGraph, AdjacenceListsGRAPH* queryGraph, int v, int w, int queryVertexId);
	static bool degreeFilter(AdjacenceListsGRAPH* queryGraph, AdjacenceListsGRAPH* dataGraph, int u, int v);

	static void computeDTable(AdjacenceListsGRAPH* queryGraph, int queryVertex, AdjacenceListsGRAPH* dataGraph, std::map<int, vector<int>* >& candidates, 
	std::queue<int> & startingQueryVertexCandidates);

	static void buildLabelRootMap(AdjacenceListsGRAPH & graph);
	static void loadContainmentGraph(vector<AdjacenceListsGRAPH> & hyperGraphVector, std::ifstream & containmentGraphFile);

};

#endif