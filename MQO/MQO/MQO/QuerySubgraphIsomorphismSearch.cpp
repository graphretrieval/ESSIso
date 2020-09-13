#include"QuerySubgraphIsomorphismSearch.h"
#include"AdjacenceListsGRAPH_IO.h"
#include<iostream>
#include <chrono>
#include"GlobalConstant.h"

using namespace std;



void QuerySubgraphIsomorphismSearch::subgraphIsomorphismSearch(AdjacenceListsGRAPH * pDataGraph, AdjacenceListsGRAPH * pQueryGraph, vector<vector<int>> * pStorageMappings) {

	startTime = std::chrono::steady_clock::now();
	candidates = std::map<int, vector<int> >();
	inverseEmbedding = std::map<int, int>();
	delete[] embedding;
	dataGraph = pDataGraph;
	queryGraph = pQueryGraph;
	storageMappings = pStorageMappings;
	/*
	 * As both the query graph and data graph are small graph, we only allow limit number embeddings returned here. 
	 */
	enoughtEmbeddingsNumber = 3;
	enoughtEmbeddingsFound = false;

	embedding = new int[queryGraph->getNumberOfVertexes()];
	for (int i = 0; i<queryGraph->getNumberOfVertexes(); i++) {
		embedding[i] = -1;
	}

	filterCandidates();

	if (candidates.size() != queryGraph->getNumberOfVertexes()) {
		return;
	}

	isomorphismSearch();

}

void QuerySubgraphIsomorphismSearch::filterCandidates() {
	// std::cout << "filterCandidates" << std::endl;
	map<int, vector<int>>::iterator candidateSetsIterator;

	vector<AdjacenceListsGRAPH::Vertex> * queryVertexLists = queryGraph->getVertexList();
	map<int, vector<int>> * labelDataVertexList = dataGraph->getLabelVertexList();

	for (vector<AdjacenceListsGRAPH::Vertex>::iterator queryVertexIterator = queryVertexLists->begin(); queryVertexIterator != queryVertexLists->end(); queryVertexIterator++) {
		/*
		* Only consider not matched vertices
		*/
		candidateSetsIterator = labelDataVertexList->find(queryVertexIterator->label);
		if (candidateSetsIterator != labelDataVertexList->end()) {
			candidates.insert(std::pair<int, std::vector<int> >(queryVertexIterator->id, candidateSetsIterator->second));
		}
	}
}


void QuerySubgraphIsomorphismSearch::isomorphismSearch() {
	// std::cout <<"isomorphismSearch" << std::endl;
	if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count() >= GlobalConstant::timeOut) 
		throw std::runtime_error("Timeout");
	if (enoughtEmbeddingsFound) {
		return;
	}
	// std::cout <<"isomorphismSearch 1 " << std::endl;
	if (inverseEmbedding.size() == queryGraph->getNumberOfVertexes()) {
		// storageMappings->push_back(vector<int>());
		// vector<int> & newEmbedding = storageMappings->at(storageMappings->size() - 1);
		vector<int>newEmbedding;
		for (int vertexIndex = 0; vertexIndex < queryGraph->getNumberOfVertexes(); vertexIndex++) {
			newEmbedding.push_back(embedding[vertexIndex]);
		}
		storageMappings->push_back(newEmbedding);
		if (storageMappings->size() >= enoughtEmbeddingsNumber) {
			enoughtEmbeddingsFound = true;
		}
		return;
	}
	// std::cout <<"isomorphismSearch 2" << std::endl;

	AdjacenceListsGRAPH::Vertex u = nextQueryVertex();
	// std::cout <<"isomorphismSearch 3" << std::endl;

	vector<int> candidates_u = candidates.find(u.id)->second;
	// std::cout <<"isomorphismSearch 4" << std::endl;

	// For each v in C(u)
	for (vector<int>::iterator v = candidates_u.begin(); v != candidates_u.end(); v++) {
		// std::cout <<"isomorphismSearch 5 " << u.id << " " << *v << std::endl;

		if (enoughtEmbeddingsFound) {
			return;
		}
		//refine candidate
		// std::cout <<"isomorphismSearch 6 " << u.id << " " << *v << std::endl;

		if (!refineCandidates(u, *v)) {
			continue;
		}
		// std::cout <<"isomorphismSearch 7 " <<  u.id << " " << *v << std::endl;

		if (isJoinable(u.id, *v)) {
			updateState(u.id, *v);
			isomorphismSearch();
			restoreState(u.id, *v);
		}
	}
}

AdjacenceListsGRAPH::Vertex QuerySubgraphIsomorphismSearch::nextQueryVertex() {
	for (int i = 0; i<queryGraph->getNumberOfVertexes(); i++) {
		if (embedding[queryGraph->getDFSTraversalOrder()->at(i)] == -1) {
			return queryGraph->getDFSTraversalOrder()->at(i);
		}
	}
}

void QuerySubgraphIsomorphismSearch::updateState(int u, int v) {
	embedding[u] = v;
	inverseEmbedding.insert(std::pair<int, int>(v, u));
}

void QuerySubgraphIsomorphismSearch::restoreState(int u, int v) {
	embedding[u] = -1;
	inverseEmbedding.erase(v);
}


bool  QuerySubgraphIsomorphismSearch::isJoinable(int u, int v) {

	AdjacenceListsGRAPH::adjIterator adjIterator(queryGraph, u);

	for (AdjacenceListsGRAPH::link t = adjIterator.begin(); !adjIterator.end(); t = adjIterator.next()) {
		// std::cout << "isjoin " << t->v << " " << queryGraph->getNumberOfVertexes() << " " << embedding[t->v] << std::endl;
		if (embedding[t->v] != -1) {
			// u has an edge with query vertex t->v which has already been matched
			if (dataGraph->edge(v, embedding[t->v])) {
				continue;
			}
			else {
				return false;
			}
		}
	}
	return true;
}
bool QuerySubgraphIsomorphismSearch::refineCandidates(AdjacenceListsGRAPH::Vertex & u, const int & v) {
	// std::cout << "refineCandidates" << std::endl;
	if (inverseEmbedding.find(v) != inverseEmbedding.end()) {
		// std::cout << "false1" << std::endl;
		return false;
	}
	// The candidates whose degree is smaller than the query vertex's will be filtered
	if (!degreeFilter(u.id, v)) {
		// std::cout << "false2" << std::endl;
		return false;
	}
	return true;
}

/* u is the query vertex id, v is the data graph vertex id */
bool QuerySubgraphIsomorphismSearch::degreeFilter(int u, int v) {

	// std::cout << "degreeFilter" << std::endl;

	AdjacenceListsGRAPH::Vertex queryVertex = queryGraph->getVertexByVertexId(u);
	// std::cout << "degreeFilter1" << std::endl;
	// std::cout << dataGraph->vertexList.size() << std::endl;
	// if (v==dataGraph->vertexList.size()) {
	// 	for (auto i : dataGraph->vertexList) std::cout << i.id << " ";
	// 	std::cout << std::endl;
	// 	for (auto i : dataGraph->labelVertexList) {
	// 		std::cout << i.first << ":";
	// 		for (auto j : i.second) std::cout << j << " ";
	// 		std::cout << std::endl;
	// 	}
	// }
	AdjacenceListsGRAPH::Vertex dataVertex = dataGraph->getVertexByVertexId(v);
	// std::cout << "degreeFilter2" << std::endl;

	for (std::map<int, std::vector<int>>::iterator labelVertexListIterator = queryVertex.labelVertexList.begin(); labelVertexListIterator != queryVertex.labelVertexList.end(); labelVertexListIterator++) {
		if (dataVertex.labelVertexList.find(labelVertexListIterator->first) == dataVertex.labelVertexList.end()) {
			// std::cout << "false1" << std::endl;
			return false;
		}
		else if (labelVertexListIterator->second.size() > dataVertex.labelVertexList.find(labelVertexListIterator->first)->second.size()) {
			// std::cout << "false2" << std::endl;
			return false;
		}
	}
	return true;
}


void QuerySubgraphIsomorphismSearch::showEmbedding() {
	std::cout << "{";
	for (int i = 0; i < queryGraph->getNumberOfVertexes(); i++) {
		cout << "*: " << i << "->" << embedding[i] << " , ";
	}
	cout << "}" << endl;
}
