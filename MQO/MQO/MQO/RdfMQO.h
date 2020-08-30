#pragma once
#ifndef RDF_MQO_DEF_H
#define RDF_MQO_DEF_H

#include"AdjacenceListsGraph.h"
#include<vector>

class RdfMQO {

private:

	/*
	* datasets
	*/
	std::vector<AdjacenceListsGRAPH> queryGraphVector;

	AdjacenceListsGRAPH dataGraph;

	std::vector<std::vector<int>> queryGroups;

public:

	RdfMQO(std::vector<AdjacenceListsGRAPH> & queryGraphVector, AdjacenceListsGRAPH & dataGraph, std::vector<std::vector<int>> & queryGroups);

	void kMeansGroupQueries();


	void refineQueryGroups();
};

#endif