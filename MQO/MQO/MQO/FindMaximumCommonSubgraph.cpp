#include"FindMaximumCommonSubgraph.h"
#include "FindMaximumConnectedClique.h"
#include "GlobalConstant.h"
#include <chrono>


#include<map>


using namespace std;

//#define DEBUG

void FindMaximumCommonSubgraph::computeMaximumCommonSubgraph(AdjacenceListsGRAPH * graph1, AdjacenceListsGRAPH * graph2, AdjacenceListsGRAPH * mcsGraph) {
	std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
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
		// if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count() >= GlobalConstant::mcsTimeOut) 
		// 	throw std::runtime_error("Timeout");
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
		return;
	}

	bool connected = false;
	bool ** productGraph = new bool*[numberOfVertex];
	for (int i = 0; i < numberOfVertex; i++) {
		// if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count() >= GlobalConstant::mcsTimeOut) 
		// 	throw std::runtime_error("Timeout");
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

		// if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count() >= GlobalConstant::mcsTimeOut) 
		// 	throw std::runtime_error("Timeout");
		const std::pair<int, int> & e1 = vertexIterator->second.first;
		const std::pair<int, int> & e2 = vertexIterator->second.second;

		map<int, std::pair<std::pair<int, int>, std::pair<int, int>>>::iterator innerVertexIterator = vertexIterator;
		innerVertexIterator++;

		for (; innerVertexIterator != productGraphVertexList.end(); innerVertexIterator++) {

			// if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count() >= GlobalConstant::mcsTimeOut) 
			// 	throw std::runtime_error("Timeout");
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

			if (connect_1 == connect_2) {
				productGraph[vertexIterator->first][innerVertexIterator->first] = true;
				productGraph[innerVertexIterator->first][vertexIterator->first] = true;
			}
		}
	}
#ifdef DEBUG
	printProductGraph(productGraph, numberOfVertex);
#endif
	/*
	* call maximal clique detection algorithm
	*/
	int * maximumClique;
	int maximumCliqueSize;
	cout << "findMaximumConnectedClique " << endl;
	// try {
	FindMaximumConnectedClique::findMaximumConnectedClique(productGraph, numberOfVertex, maximumClique, maximumCliqueSize);
	// } 
	// catch(std::runtime_error& e) {
	// 	std::cout << e.what() << std::endl;
	// 	return;
	// }
	cout << maximumCliqueSize << endl;
	/*
	 * @efficency
	 * If the maximum clique size is less than 3, we just return. cause a small(or no) mcs will be computed
	 */
	if (maximumCliqueSize < GlobalConstant::G_MINIMUM_NUMBER_MCS_VERTEX) {
		return;
	}

#ifdef DEBUG
	printMaximalClique(maximumClique, maximumCliqueSize);
#endif

	/*
	* construct a graph based on the clique result
	* iterate each node in the product graph == iterate each edge of the common subgraph of graph1 (graph2)
	*/
	mcsGraph->clear();
	map<int, int> subIdToMCSGraphVertex;
	map<int, int>::iterator idIteratorSource, idIteratorDesti;
	int newIdSouce, newIdDesti;
	int numberOfCliqueVertex = 0;

	for (int j = 0; j < maximumCliqueSize; j++) {
		// if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count() >= GlobalConstant::mcsTimeOut) 
		// 	throw std::runtime_error("Timeout");
		map<int, std::pair<std::pair<int, int>, std::pair<int, int>>>::iterator edgePair = productGraphVertexList.find(maximumClique[j]);

		idIteratorSource = subIdToMCSGraphVertex.find(edgePair->second.second.first);
		if (idIteratorSource == subIdToMCSGraphVertex.end()) {
			subIdToMCSGraphVertex.insert(std::pair<int, int>(edgePair->second.second.first, numberOfCliqueVertex));

			AdjacenceListsGRAPH::Vertex vertex(numberOfCliqueVertex, graph2->getVertexByVertexId(edgePair->second.second.first).label);
			mcsGraph->insert(vertex);
			newIdSouce = numberOfCliqueVertex;
			numberOfCliqueVertex++;
		}
		else {
			newIdSouce = idIteratorSource->second;
		}

		idIteratorDesti = subIdToMCSGraphVertex.find(edgePair->second.second.second);
		if (idIteratorDesti == subIdToMCSGraphVertex.end()) {
			subIdToMCSGraphVertex.insert(std::pair<int, int>(edgePair->second.second.second, numberOfCliqueVertex));

			AdjacenceListsGRAPH::Vertex vertex(numberOfCliqueVertex, graph2->getVertexByVertexId(edgePair->second.second.second).label);
			mcsGraph->insert(vertex);
			newIdDesti = numberOfCliqueVertex;
			numberOfCliqueVertex++;
		}
		else {
			newIdDesti = idIteratorDesti->second;
		}

		AdjacenceListsGRAPH::Edge edge(newIdSouce, newIdDesti, 0);
		mcsGraph->insert(edge, true);
	}

	delete[] maximumClique;
	/*
	* Make sure the MCS is a connected graph
	*/
	mcsGraph->buildDFSTraversalOrder();
	if (mcsGraph->getDFSTraversalOrder()->size() < mcsGraph->getNumberOfVertexes()) {
		/*
		 * If the maximum clique doesn't generate a connected graph, we will extract a maximum graph from it
		 */
		std::map<int, int> mcsVertexIndexMap;
		int * mcsVertexLabels = new int[mcsGraph->getDFSTraversalOrder()->size()];
		int mcsVertexId;
		for (int i = 0; i < mcsGraph->getDFSTraversalOrder()->size(); i++) {
			// if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count() >= GlobalConstant::mcsTimeOut) 
			// 	throw std::runtime_error("Timeout");
			mcsVertexId = mcsGraph->getDFSTraversalOrder()->at(i);
			mcsVertexIndexMap.insert(std::pair<int, int>(mcsVertexId, i));
			mcsVertexLabels[i] = mcsGraph->getVertexAddressByVertexId(mcsVertexId)->label;
		}

		std::vector<AdjacenceListsGRAPH::Edge> largerConnectedMcsEdges;
		for (std::vector<AdjacenceListsGRAPH::Edge>::iterator mcsEdgeIterator = mcsGraph->getEdgeList()->begin(); mcsEdgeIterator != mcsGraph->getEdgeList()->end(); mcsEdgeIterator++) {
			// if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count() >= GlobalConstant::mcsTimeOut) 
			// 	throw std::runtime_error("Timeout");
			std::map<int, int>::iterator mcsSourceVertexIndexIterator = mcsVertexIndexMap.find(mcsEdgeIterator->source);
			std::map<int, int>::iterator mcsDestVertexIndexIterator = mcsVertexIndexMap.find(mcsEdgeIterator->destination);
			if (mcsSourceVertexIndexIterator != mcsVertexIndexMap.end() && mcsDestVertexIndexIterator != mcsVertexIndexMap.end()) {
				largerConnectedMcsEdges.push_back(AdjacenceListsGRAPH::Edge(mcsSourceVertexIndexIterator->second, mcsDestVertexIndexIterator->second));
			}
		}
		mcsGraph->clear();
		for (int i = 0; i < mcsVertexIndexMap.size(); i++) {
			AdjacenceListsGRAPH::Vertex vertex(i, mcsVertexLabels[i]);
			mcsGraph->insert(vertex);
		}
		for (std::vector<AdjacenceListsGRAPH::Edge>::iterator largerMCSEdgeIter = largerConnectedMcsEdges.begin(); largerMCSEdgeIter != largerConnectedMcsEdges.end(); largerMCSEdgeIter++) {
			mcsGraph->insert(AdjacenceListsGRAPH::Edge(largerMCSEdgeIter->source, largerMCSEdgeIter->destination, 0), true);
		}
		mcsGraph->buildDFSTraversalOrder();

		delete[] mcsVertexLabels;
	}
	
	/*
	 * restrict the size
	 */
	if (mcsGraph -> getNumberOfVertexes() <= GlobalConstant::G_MINIMUM_NUMBER_MCS_VERTEX) {
		mcsGraph -> clear();
	}

	/*
	* release memoery
	*/
	for (int i = 0; i < numberOfVertex; i++) {
		delete[] productGraph[i];
	}
	delete[] productGraph;
}

void FindMaximumCommonSubgraph::printProductGraph(bool ** productGraph, int size) {
	cout << "The product graph is:" << endl;
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			cout << productGraph[i][j] << " ";
		}
		cout << endl;
	}
}

void FindMaximumCommonSubgraph::printMaximalClique(int * maximumClique, int maximumCliqueSize) {
	cout << "The maximal clique size is:" << maximumCliqueSize << endl;
	for (int i = 0; i < maximumCliqueSize; i++) {
		cout << maximumClique[i] << " ";
	}
	cout << endl;
}