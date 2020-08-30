#include"QuerySubgraphIsomorphismSearch.h"
#include"AdjacenceListsGRAPH_IO.h"
#include<iostream>
#include <chrono>
#include"GlobalConstant.h"

using namespace std;



void QuerySubgraphIsomorphismSearch::subgraphIsomorphismSearch(AdjacenceListsGRAPH * pDataGraph, AdjacenceListsGRAPH * pQueryGraph, vector<vector<int>> * pStorageMappings) {

	startTime = std::chrono::steady_clock::now();
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

	candidates = std::map<int, vector<int>* >();
	inverseEmbedding = std::map<int, int>();
	delete[] embedding;

}

void QuerySubgraphIsomorphismSearch::filterCandidates() {

	map<int, vector<int>>::iterator candidateSetsIterator;

	vector<AdjacenceListsGRAPH::Vertex> * queryVertexLists = queryGraph->getVertexList();
	map<int, vector<int>> * labelDataVertexList = dataGraph->getLabelVertexList();

	for (vector<AdjacenceListsGRAPH::Vertex>::iterator queryVertexIterator = queryVertexLists->begin(); queryVertexIterator != queryVertexLists->end(); queryVertexIterator++) {
		/*
		* Only consider not matched vertices
		*/
		candidateSetsIterator = labelDataVertexList->find(queryVertexIterator->label);
		if (candidateSetsIterator != labelDataVertexList->end()) {
			candidates.insert(std::pair<int, std::vector<int> *>(queryVertexIterator->id, &candidateSetsIterator->second));
		}
	}
}


void QuerySubgraphIsomorphismSearch::isomorphismSearch() {
	if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count() >= GlobalConstant::timeOut) 
		throw std::runtime_error("Timeout");
	if (enoughtEmbeddingsFound) {
		return;
	}

	if (inverseEmbedding.size() == queryGraph->getNumberOfVertexes()) {
		storageMappings->push_back(vector<int>());
		vector<int> & newEmbedding = storageMappings->at(storageMappings->size() - 1);
		for (int vertexIndex = 0; vertexIndex < queryGraph->getNumberOfVertexes(); vertexIndex++) {
			newEmbedding.push_back(embedding[vertexIndex]);
		}
		if (storageMappings->size() >= enoughtEmbeddingsNumber) {
			enoughtEmbeddingsFound = true;
		}
		return;
	}
	AdjacenceListsGRAPH::Vertex u = nextQueryVertex();
	vector<int> * candidates_u = candidates.find(u.id)->second;

	// For each v in C(u)
	for (vector<int>::iterator v = candidates_u->begin(); v != candidates_u->end(); v++) {
		if (enoughtEmbeddingsFound) {
			return;
		}
		//refine candidate
		if (!refineCandidates(u, *v)) {
			continue;
		}
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
	if (inverseEmbedding.find(v) != inverseEmbedding.end()) {
		return false;
	}
	// The candidates whose degree is smaller than the query vertex's will be filtered
	if (!degreeFilter(u.id, v)) {
		return false;
	}
	return true;
}

/* u is the query vertex id, v is the data graph vertex id */
bool QuerySubgraphIsomorphismSearch::degreeFilter(int u, int v) {


	AdjacenceListsGRAPH::Vertex queryVertex = queryGraph->getVertexByVertexId(u);
	AdjacenceListsGRAPH::Vertex dataVertex = dataGraph->getVertexByVertexId(v);


	for (std::map<int, std::vector<int>>::iterator labelVertexListIterator = queryVertex.labelVertexList.begin(); labelVertexListIterator != queryVertex.labelVertexList.end(); labelVertexListIterator++) {
		if (dataVertex.labelVertexList.find(labelVertexListIterator->first) == dataVertex.labelVertexList.end()) {
			return false;
		}
		else if (labelVertexListIterator->second.size() > dataVertex.labelVertexList.find(labelVertexListIterator->first)->second.size()) {
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
