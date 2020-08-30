#pragma once

#include"AdjacenceListsGraph.h"


class FindMaximumCommonSubgraph {

public:

	static void computeMaximumCommonSubgraph(AdjacenceListsGRAPH * graph1, AdjacenceListsGRAPH * graph2, AdjacenceListsGRAPH * mcsGraph);

	static void printProductGraph(bool ** productGraph, int size);

	static void printMaximalClique(int * maximumClique, int maximumCliqueSize);
};

