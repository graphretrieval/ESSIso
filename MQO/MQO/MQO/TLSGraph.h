#pragma once

#include"AdjacenceListsGraph.h"
#include<vector>
#include<map>

using namespace std;

class TLSGraph{
private:

	float ** tlsGraphMatrix;

	std::vector<AdjacenceListsGRAPH> * queryGraphVector;

	std::ofstream * resultFile;

public:




public:

	TLSGraph(float ** pTlsGraphMatrix, std::vector<AdjacenceListsGRAPH> * pQueryGraphVector, std::ofstream * pResultFile);

	/*
	 * Build a TLS graph, each matrix element is the number of common TLS of two graphs
	 * @param similarityIndex 0 represents tlsSimilarity, 1 represents jaccard similarity
	 */
	void buildTLSGraph(int similarityIndex);
	
};

