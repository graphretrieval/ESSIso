#pragma once
#ifndef DEBUG_GRAPH_OVERLAP_QG_H
#define DEBUG_GRAPH_OVERLAP_QG_H

#include"AdjacenceListsGraph.h"
#include<fstream>

/*
 * Temparary query generator for debugging purpose
 */
class DebugGraphOverlapQG {

private:

	AdjacenceListsGRAPH * dataGraph;
	std::ofstream * resultFile;
	
	std::ofstream * resultFile2;


	int queryGraphIndex;

public:

	DebugGraphOverlapQG(AdjacenceListsGRAPH * pDataGraph, std::ofstream * pResultFile, std::ofstream * pResultFile2);

	DebugGraphOverlapQG(AdjacenceListsGRAPH * pDataGraph, std::ofstream * pResultFile);

	void generateQueries();

private:

	void outputQuery(std::vector<int> & vertexIdVector, std::vector<int> & vertexLabelVector);


};

#endif