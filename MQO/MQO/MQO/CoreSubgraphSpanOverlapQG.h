#pragma once
#ifndef CORE_SUBGRAPH_SPAN_QUERY_GENERATOR_H
#define CORE_SUBGRAPH_SPAN_QUERY_GENERATOR_H

#include"AdjacenceListsGraph.h"
#include<fstream>
#include<random>
#include<vector>
#include<set>
#include<map>

class CoreSubgraphSpanOverlapQG {

private:
	AdjacenceListsGRAPH * dataGraph;
	int numberOfQueries;
	/*
	* 0 subgraph
	* 1 path
	*/
	int queryType;
	std::ofstream * resultFile;

private:

	std::default_random_engine generator;

	int queryGraphIndex;

	int BASE_QUERY_SIZE;
	int MAXIMUM_QUERY_SIZE;
	int QUERY_GROUP_SIZE;
	int NUMBER_CORES;

public:
	CoreSubgraphSpanOverlapQG();
	CoreSubgraphSpanOverlapQG(AdjacenceListsGRAPH * pDataGraph, int pQueryType, std::ofstream * pResultFile, int pNumberOfCores, int pQueryGroupSize);
	~CoreSubgraphSpanOverlapQG();

	void generateQueries();

private:
	void generateSubgraphs(std::vector<int> & coreVerticesVector);
	void generatePaths(std::vector<int> & coreVerticesVector);


	void outputQuery(std::vector<int> & vertexIdVector, std::vector<int> & vertexLabelVector);

};

#endif