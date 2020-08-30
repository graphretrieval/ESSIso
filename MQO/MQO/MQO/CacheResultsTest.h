#pragma once
#include"ComboLinkedLists.h"
#include"CacheEmbeddingNode.h"
#include"TurboIso_CacheResults.h"
#include<vector>
#include<set>
#include<iostream>

class CacheResultsTest {

private:


	std::ofstream * resultFile;

	/*
	* Used to save the number of founded embeddings
	*/
	std::vector<int> numberOfEmbeddings;


	std::vector<std::vector<std::vector<int>>> trivialEmbeddingLists;

	std::vector<std::vector<CacheEmbeddingNode *>> * batchResultCaches;

	/*
	* datasets
	*/
	std::vector<AdjacenceListsGRAPH> * queryGraphVector;


	AdjacenceListsGRAPH * dataGraph;

	TurboIso_CacheResults * turboIso_CacheResults;

private:

	long long trivialEmbeddingListMSize;
	long long * linkedListsMSize;
	double * linkedListRetriTime;

	CacheEmbeddingNode ** comboVertexMapping;

public:

	CacheResultsTest(AdjacenceListsGRAPH * pDataGraph, std::vector<AdjacenceListsGRAPH> * pQueryGraphVector, std::ofstream * pResultFile);

	void execute();

private:

	void computeMemory(int queryGraphId, int parWidthIndex);


	void saveCLLResults(int parWidth, int queryGraphId, std::vector<std::vector<CacheEmbeddingNode *>> * resultCaches);

	void computeCLLRetrivingPerfor(int queryGraphId, std::vector<std::vector<CacheEmbeddingNode *>> * cachedLinkedLists, double *  linkedListRetriTime);

	void recursiveEnumLinkedLists(int comboVertexOrder, AdjacenceListsGRAPH * queryGraph);
};

