#include"FindAllMaximalCommonSubgraphs.h"
#include "FindMaximalDisjointConnCliques.h"
#include "GlobalConstant.h"


#include<map>


using namespace std;


//#define DEBUG
int FindAllMaximalCommonSubgraphs::computeAllMaximalCommonSubgraphs(AdjacenceListsGRAPH * graph1, AdjacenceListsGRAPH * graph2, std::vector<AdjacenceListsGRAPH> * mcsGraphs) {

	/*
	* Each vertex in the product graph represents a edge-pair of graph2
	*/
	map<int, std::pair<std::pair<int, int>, std::pair<int, int>>> productGraphVertexList;

	int numberOfVertex = 0;
	std::map<std::pair<int, int>, std::vector<std::pair<int, int>>> * labelEdgeList1 = graph1->getVertexLabelsEdgeList();
	std::map<std::pair<int, int>, std::vector<std::pair<int, int>>> * labelEdgeList2 = graph2->getVertexLabelsEdgeList();

	/*****************************************************
	* Build the compativity graph
	****************************************************/
	for (std::map<std::pair<int, int>, std::vector<std::pair<int, int>>>::iterator labelEdgeIterator1 = labelEdgeList1->begin(); labelEdgeIterator1 != labelEdgeList1->end(); labelEdgeIterator1++) {
		std::map<std::pair<int, int>, std::vector<std::pair<int, int>>>::iterator labelEdgeIterator2 = labelEdgeList2->find(labelEdgeIterator1->first);
		if (labelEdgeIterator2 != labelEdgeList2->end()) {
			for (unsigned int i = 0; i<labelEdgeIterator1->second.size(); i++) {
				for (unsigned int j = 0; j<labelEdgeIterator2->second.size(); j++) {

					productGraphVertexList.insert(std::pair<int, std::pair<std::pair<int, int>, std::pair<int, int>>>(numberOfVertex, std::pair<std::pair<int, int>, std::pair<int, int>>(labelEdgeIterator1->second[i], labelEdgeIterator2->second[j])));

					numberOfVertex++;
				}
			}
		}
	}

	/*
	* @efficency
	* This will significantly increase the mcs building time ! Set some dummy threshold to limit the mcs size.
	* Cause there are so many small mcses
	*/
	if (numberOfVertex <= GlobalConstant::G_MINIMUM_NUMBER_MCS_VERTEX) {
		return 0;
	}

	bool connected = false;
	bool ** productGraph = new bool*[numberOfVertex];
	for (int i = 0; i < numberOfVertex; i++) {
		productGraph[i] = new bool[numberOfVertex];
		for (int j = 0; j < numberOfVertex; j++) {
			if (i == j) {
				productGraph[i][j] = true;
			}
			else {
				productGraph[i][j] = false;
			}
		}
	}


	for (map<int, std::pair<std::pair<int, int>, std::pair<int, int>>>::iterator vertexIterator = productGraphVertexList.begin(); vertexIterator != productGraphVertexList.end(); vertexIterator++) {

		const std::pair<int, int> & e1 = vertexIterator->second.first;
		const std::pair<int, int> & e2 = vertexIterator->second.second;

		map<int, std::pair<std::pair<int, int>, std::pair<int, int>>>::iterator innerVertexIterator = vertexIterator;
		innerVertexIterator++;

		for (; innerVertexIterator != productGraphVertexList.end(); innerVertexIterator++) {

			const std::pair<int, int> & e3 = innerVertexIterator->second.first;
			const std::pair<int, int> & e4 = innerVertexIterator->second.second;

			if (e1 == e3 || e2 == e4) {
				continue;
			}

			/*
			* Check connectivity
			*/
			int connect_1 = 0, connect_2 = 0;
			if (e1.first == e3.first) {
				connect_1 = 1;
			}
			else if (e1.second == e3.first) {
				connect_1 = 2;
			}
			else if (e1.second == e3.second) {
				connect_1 = 3;
			}
			if (e2.first == e4.first) {
				connect_2 = 1;
			}
			else if (e2.second == e4.first) {
				connect_2 = 2;
			}
			else if (e2.second == e4.second) {
				connect_2 = 3;
			}
			
			if (connect_1 != 0 && connect_2 != 0) {
				if (connect_1 == connect_2) {
					productGraph[vertexIterator->first][innerVertexIterator->first] = true;
					productGraph[innerVertexIterator->first][vertexIterator->first] = true;
				}
			}
		}
	}
#ifdef DEBUG
	printProductGraph(productGraph, numberOfVertex);
#endif

	/*
	* call maximal clique detection algorithm
	*/
	std::vector<std::vector<int>> similiarQueryGroups;
	FindMaximalDisjointConnCliques::findMaximalDisjointConnCliques(productGraph, numberOfVertex, similiarQueryGroups);

	int numberOfMCS = 0;
	for (std::vector<vector<int>>::iterator groupIterator = similiarQueryGroups.begin(); groupIterator != similiarQueryGroups.end(); groupIterator++) {
		if (groupIterator->size() < GlobalConstant::G_MINIMUM_NUMBER_MCS_VERTEX) {
			continue;
		}
		numberOfMCS++;
	}


	/*
	* release memoery
	*/
	for (int i = 0; i < numberOfVertex; i++) {
		delete[] productGraph[i];
	}
	delete[] productGraph;

	return numberOfMCS;
}

void FindAllMaximalCommonSubgraphs::printProductGraph(bool ** productGraph, int size) {
	cout << "The product graph is:" << endl;
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			cout << productGraph[i][j] << " ";
		}
		cout << endl;
	}
}

void FindAllMaximalCommonSubgraphs::printMaximalClique(int * maximumClique, int maximumCliqueSize) {
	cout << "The maximal clique size is:" << maximumCliqueSize << endl;
	for (int i = 0; i < maximumCliqueSize; i++) {
		cout << maximumClique[i] << " ";
	}
	cout << endl;
}