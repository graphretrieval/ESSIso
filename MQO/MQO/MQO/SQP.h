#ifndef SQO_DEF_H
#define SQO_DEF_H


#include"AdjacenceListsGraph.h"
#include"AdjacenceListsGRAPH_IO.h"
#include"TLSGraph.h"
#include"MQO.h"
#include"PCMBuilder.h"
#include"TurboIso.h"
#include"TurboIsoBoosted.h"
#include"TurboIsoBoostedMQO.h"
#include"EmBoost.h"
#include<vector>
#include<iostream>
#include<list>
#include<array>

class SQP{

public: 
	int totalCall = 0;
	
private:

	/*
	 * subgraph isomorphism algorithms
	 */
	TurboIsoBoosted * turboIsoBoosted;

	TurboIso * turboIso;

	EmBoost * emBoost;

private:

	/*
	 * datasets
	 */
	std::vector<AdjacenceListsGRAPH> * queryGraphVector;
	
	AdjacenceListsGRAPH * dataGraph;

	AdjacenceListsGRAPH * queryGraph;

	std::ofstream * resultFile;

	/*
	 * Used to save the number of founded embeddings
	 */
	std::vector<int> numberOfEmbeddings;

	double timeClapse;


public:
	SQP();
	SQP(std::vector<AdjacenceListsGRAPH> * dataGraphVector, std::vector<AdjacenceListsGRAPH> * pQueryGraphVector, std::ofstream * pResultFile);
	~SQP();

	void queryProcessing();


};

#endif 