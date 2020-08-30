#pragma once

/*
 * Given the MLS matrix, we find all the maximal cliques from the matrix
 * Each clique is a group of queries grouped together.
 * Notice that, in each group. we don't allow containment-relationship vertices.
 */
#include<vector>
#include"AdjacenceListsGraph.h"
#include"PCM_Node.h"


class PCM_Node;


class FindAllMaximalCliques
{
private:

	class nodeSet {

	public:
		int N;
		int *node;
		int size;

		nodeSet(void);
		~nodeSet(void);
		void nodeInit(int NInput);
		void add(int nodeA);
		void remove(void);
		void print(void);
	};


public:
	FindAllMaximalCliques();
	FindAllMaximalCliques(std::vector<AdjacenceListsGRAPH> * pQueryGraphVector);
	~FindAllMaximalCliques();
	
	void findMaxclique(const bool **, std::vector<std::vector<int>> * pSimiliarQueryGroups, std::vector<PCM_Node> * pPatternContainmentMap);

private:
	const bool ** graph;
	int N;
	nodeSet compsub;
	std::vector<std::vector<int>> * cliques;
	std::vector<AdjacenceListsGRAPH> * queryGraphVector;
	std::vector<PCM_Node> * patternContainmentMap;
private:
	void bkv2(int* pOld, int ne, int ce);
	void addToCliques();
};

