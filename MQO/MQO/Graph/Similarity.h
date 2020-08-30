#pragma once
#ifndef SIMILARITY_H
#define SIMILARITY_H

#include"AdjacenceListsGraph.h"

class Similarity {

public:

	static float tlsSimilarity(AdjacenceListsGRAPH * graph1, AdjacenceListsGRAPH * graph2);

	static float jaccardSimilarity(AdjacenceListsGRAPH * graph1, AdjacenceListsGRAPH * graph2);

private:

	/*
	* Give a list of Tls instances, return the maximum connected tls size. two tls are connected if they share any common vertex
	*/
	static int computeMaxConnectedTlsSize(std::vector<std::vector<int>> * coveredTlsInstances);
	
};

#endif