#pragma once

#include<vector>
#include<string>
#include<map>
#include<list>
#include<set>
#include"QueryGraphIsomorphismTest.h"
#include"QuerySubgraphIsomorphismSearch.h"
#include"AdjacenceListsGraph.h"
#include"PCM_Node.h"

class FindAllMaximalCliques;


class PCMBuilder{

private:

	QueryGraphIsomorphismTest graphIsomorphism;

	QuerySubgraphIsomorphismSearch subgraphIsomorphism;

	std::vector<vector<int>> similiarQueryGroups;

private:

	std::vector<AdjacenceListsGRAPH> * queryGraphVector;

	std::vector<PCM_Node> * patternContainmentMap;

	std::vector<AdjacenceListsGRAPH> * newlyGeneratedGraphVector;

	float ** tlsGraphMatrix;

	std::ostream * resultFile;

public:

	PCMBuilder(float ** pTlsGraphMatrix, std::vector<PCM_Node> * pPatternContainmentMap, std::vector<AdjacenceListsGRAPH> * pQueryGraphVector, std::vector<AdjacenceListsGRAPH> * pNewlyGeneratedGraphVector, std::ostream * pResultFile);

	~PCMBuilder();

	void execute();

private:

	void hideIsomorphicQueries();

	void buildContainmentRelations();

	void detectCommonSubgraphs(vector<int> & groupQuery);
	/*
	* 1. Do the transitive reduction
	* 2. Build mappingVerticesSet helper variables from mappingVerticesList. We used a list and a set to save the mapping informations
	*/
	void formatPCM();

	void showPCM();

	void printSimilarGroups();
};
