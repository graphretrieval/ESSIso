#pragma once
#ifndef SIMI_METHOD_TEST_H
#define SIMI_METHOD_TEST_H

#include"PCMBuilder.h"
#include"TLSGraph.h"
#include<vector>
#include<set>
#include<iostream>

struct simi_greater
{
	template<class T>
	bool operator()(T const &a, T const &b) const { return a > b; }
};


class SimiMethodTest {

private:

	std::vector<AdjacenceListsGRAPH> * queryGraphVector;

	std::ofstream * resultFile;

	/*
	* used to store the Triple Label Sequence Matrix
	*/
	float ** tlsGraphMatrix;

	float ** jaccardGraphMatrix;

	/*
	 * we need to sort this list by similarity, the int is the id (computed by i and j)
	 */
	std::vector<std::pair<float, int>> tlsSimiList;

	std::vector<std::pair<float, int>> jaccardSimiList;

	/*
	 * used to store the mcs for each query pair
	 * we use a int to represent the edge number of the mcs
	 */
	int ** mcsStoreMatrix;

public:

	SimiMethodTest(std::vector<AdjacenceListsGRAPH> * pQueryGraphVector, std::ofstream * pResultFile);

	~SimiMethodTest();

	void execute();
};

#endif