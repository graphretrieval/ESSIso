#pragma once
#ifndef RANDOM_QUERY_GENERATOR_H
#define RANDOM_QUERY_GENERATOR_H

#include"AdjacenceListsGraph.h"
#include<fstream>
#include<random>
#include<vector>
#include<set>
#include<map>

class RandomQG {

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

	int numberOfGraphs;

	int BASE_QUERY_SIZE;
	int MAXIMUM_QUERY_SIZE;

public:
	RandomQG();
	RandomQG(AdjacenceListsGRAPH * pDataGraph, int pQueryType, std::ofstream * pResultFile, int pNumberOfGraphs);
	~RandomQG();

	void generateQueries();

private:
	void generateSubgraphs();


};

#endif