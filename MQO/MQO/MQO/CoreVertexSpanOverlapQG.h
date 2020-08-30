#pragma once
#ifndef CORE_VERTEX_SPAN_QUERY_GENERATOR_H
#define CORE_VERTEX_SPAN_QUERY_GENERATOR_H

#include"AdjacenceListsGraph.h"
#include<fstream>
#include<random>
#include<vector>
#include<set>
#include<map>

class CoreVertexSpanOverlapQG {

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
	int MINI_NUMBER_QUERY_SIZE;
	int MAX_NUMBER_QUERY_SIZE;

	int numberOfCores;
	int DUPLICATE_DRAW_PROB;

	int queryGraphIndex;

private:
	// To not generate queries that have two few labels. we use a ratio here
	double LABEL_NO_VERTEX_NO_RATIO;

public:
	CoreVertexSpanOverlapQG();
	CoreVertexSpanOverlapQG(AdjacenceListsGRAPH * pDataGraph, int pNumberOfQueries, double pSimilarRatio, int pQueryType, std::ofstream * pResultFile);
	~CoreVertexSpanOverlapQG();

	void generateQueries();

private:
	void generateSubgraphs(std::vector<int> & coreVerticesVector);
	void generatePaths(std::vector<int> & coreVerticesVector);

	void outputQuery(std::vector<std::pair<int, AdjacenceListsGRAPH::Vertex *>> &  queryVertexArr, std::vector<std::pair<int, int>> & edgeSet);

};

#endif