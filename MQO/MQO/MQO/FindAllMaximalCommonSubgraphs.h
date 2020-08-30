#pragma once

#include"AdjacenceListsGraph.h"


class FindAllMaximalCommonSubgraphs {

public:

	/*
	 * return the numberof common subgraphs detected
	 */
	static int computeAllMaximalCommonSubgraphs(AdjacenceListsGRAPH * graph1, AdjacenceListsGRAPH * graph2, std::vector<AdjacenceListsGRAPH> * mcsGraphs);

	static void printProductGraph(bool ** productGraph, int size);

	static void printMaximalClique(int * maximumClique, int maximumCliqueSize);
};

