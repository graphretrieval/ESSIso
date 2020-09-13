#include"EmBoost.h"
#include"TimeUtility.h"
#include"MathUtility.h"
#include"AdjacenceListsGRAPH_IO.h"
#include<iostream>
#include<vector>
#include<map>
#include<set>
#include<queue>
#include <limits>
#include"GlobalConstant.h"
// #include<faiss/IndexFlat.h>
#include<algorithm>
#include <iterator>
#include"GlobalConstant.h"
#include <unordered_set>

using namespace std;


EmBoost::EmBoost(AdjacenceListsGRAPH * pDataGraph, std::vector<int> * pNumberOfEmbeddings, std::vector<std::vector<CacheEmbeddingNode *>> * pResultCaches) {
	dataGraph = pDataGraph;
	numberOfEmbeddings = pNumberOfEmbeddings;
	resultCaches = pResultCaches;

	embedding = NULL;
}

EmBoost::~EmBoost() {
	if (embedding != NULL) {
		delete embedding;
		embedding = NULL;
	}
	inverseEmbedding = std::map<int, int>();

}


void EmBoost::setParameters(AdjacenceListsGRAPH * pQueryGraph, bool pNeedToSaveCache) {

	queryGraph = pQueryGraph;
	needFormatCache = pNeedToSaveCache;

	needReturn = false;
	recursiveCallsNo = 0;
	CRRecursiveCallsNo = 0;

	if (embedding != NULL) {
		delete embedding;
		embedding = NULL;
	}
	embedding = new int [queryGraph->getNumberOfVertexes()];
	for (int i = 0; i < queryGraph->getNumberOfVertexes();  i++) {
		embedding[i] = -1; 
	}

	inverseEmbedding = std::map<int, int>();
	candidates = std::map<int, vector<int> >();
}



void EmBoost::execute() {
	embedding = new int[queryGraph->getNumberOfVertexes()];
	for (int i = 0; i<queryGraph->getNumberOfVertexes(); i++) {
		embedding[i] = -1;
	}
	filterCandidates();
	if (candidates.size() != queryGraph->getNumberOfVertexes()) {
		return;
	}
	subgraphSearch();
	}

void EmBoost::filterCandidates() {
	// faiss::IndexFlatL2 index(GlobalConstant::embeddingDimension);
	for(int i = 0; i <  dataGraph->getNumberOfVertexes(); i++) {
		for(int j = 0; j < 3; j++)
			printf("%f ", dataGraph->getEmbeddingMatrix()[i * 3 + j]);
		printf("\n");
    }
	for(int i = 0; i <  queryGraph->getNumberOfVertexes(); i++) {
		for(int j = 0; j < 3; j++)
			printf("%f ", queryGraph->getEmbeddingMatrix()[i * 3 + j]);
		printf("\n");
    }
	// index.add(dataGraph->getNumberOfVertexes(), dataGraph->getEmbeddingMatrix()); 
	long *I = new long[GlobalConstant::numberOfCandidates * queryGraph->getNumberOfVertexes()];
    float *D = new float[GlobalConstant::numberOfCandidates * queryGraph->getNumberOfVertexes()];

	// index.search(queryGraph->getNumberOfVertexes(), queryGraph->getEmbeddingMatrix(), GlobalConstant::numberOfCandidates, D, I);

	vector<vector<int>> filterFaissCandidateList;
	for (int i = 0; i< queryGraph->getNumberOfVertexes();i++) {
		vector<int> filterFaissCandidate;
		for (int j =0 ;j<GlobalConstant::numberOfCandidates;j++) {
			// if (D[i*GlobalConstant::numberOfCandidates+j] < GlobalConstant::distThreshold)  
			filterFaissCandidate.push_back(I[i*GlobalConstant::numberOfCandidates+j]);
		}
		filterFaissCandidateList.push_back(filterFaissCandidate);
	}

	// for(int i = 0; i <  queryGraph->getNumberOfVertexes(); i++) {
	// 	for(int j = 0; j < k; j++)
	// 		printf("%5ld ", I[i * k + j]);
	// 	printf("\n");
    // }

	map<int, vector<int>>::iterator candidateSetsIterator;
	vector<AdjacenceListsGRAPH::Vertex> * queryVertexLists = queryGraph->getVertexList();
	map<int, vector<int>> * labelDataVertexList = dataGraph->getLabelVertexList();

	for (vector<AdjacenceListsGRAPH::Vertex>::iterator queryVertexIterator = queryVertexLists->begin(); queryVertexIterator != queryVertexLists->end(); queryVertexIterator++) {
		/*
		* Only consider not matched vertices
		*/
		candidateSetsIterator = labelDataVertexList->find(queryVertexIterator->label);

		if (candidateSetsIterator != labelDataVertexList->end()) {
			
			cout << "Number of label candidates of " << queryVertexIterator->id << " is " << candidateSetsIterator->second.size() << endl;
			for (vector<int>::iterator it = candidateSetsIterator->second.begin(); it!= candidateSetsIterator->second.end();it++)
				cout << *it << " ";
			cout << endl;

			vector<int> combineCandidates;
			
			vector<int> filterFaissCandidate = filterFaissCandidateList.at(queryVertexIterator->id);
			cout << "Number of candidates of " << queryVertexIterator->id << " is " << filterFaissCandidate.size() << endl;
			for (vector<int>::iterator it = filterFaissCandidate.begin(); it!= filterFaissCandidate.end();it++)
				cout << *it << " ";
			cout << endl;
			
			std::sort(candidateSetsIterator->second.begin(), candidateSetsIterator->second.end());
    		std::sort(filterFaissCandidateList.begin(), filterFaissCandidateList.end());

			std::set_intersection (filterFaissCandidate.begin(), filterFaissCandidate.end(), candidateSetsIterator->second.begin(), candidateSetsIterator->second.end(), std::back_inserter(combineCandidates));
			
			unordered_set<int> m(filterFaissCandidate.begin(), filterFaissCandidate.end());
			for (auto a : candidateSetsIterator->second)
				if (m.count(a)) {
					combineCandidates.push_back(a);
					m.erase(a);
				}

			std::pair<int, std::vector<int> > tempPair = std::pair<int, std::vector<int> >(queryVertexIterator->id, candidateSetsIterator->second);
			cout << "query:" << queryVertexIterator->id << endl;
			for (vector<int>::iterator it = combineCandidates.begin(); it!=combineCandidates.end();it++){
				cout << *it << " ";
			}
			cout << endl;
			cout << "Number of final candidates of " << queryVertexIterator->id << " is " << combineCandidates.size() << endl;
			for (vector<int>::iterator it = combineCandidates.begin(); it!= combineCandidates.end();it++)
				cout << *it << " ";
			cout << endl;	

			// std::pair<int, std::vector<int> > tempPair = std::pair<int, std::vector<int> >(queryVertexIterator->id, candidateSetsIterator->second);
		
			candidates.insert(tempPair);
		}
	}
}

void EmBoost::updateState(int u, int v) {
	embedding[u] = v;
	inverseEmbedding.insert(std::pair<int, int>(v, u));
}

void EmBoost::restoreState(int u, int v) {
	embedding[u] = -1;
	inverseEmbedding.erase(v);
}


void EmBoost::subgraphSearch() {
	if (recursiveCallsNo++ >= GlobalConstant::G_enoughRecursiveCalls && GlobalConstant::G_enoughRecursiveCalls > 0) {
		// (*numberOfEmbeddings)[queryGraph->graphId] = -1;
		needReturn = true;
		return;
	}
	if ((*numberOfEmbeddings)[queryGraph->graphId] >= GlobalConstant::G_SUFFICIENT_NUMBER_EMBEDDINGS) {
		return;
	}
	if (inverseEmbedding.size() == queryGraph->getNumberOfVertexes()) {
		(*numberOfEmbeddings)[queryGraph->graphId] ++;
		return;
	}

	AdjacenceListsGRAPH::Vertex u = nextQueryVertex();
	vector<int> candidates_u = candidates.find(u.id)->second;
	// // For each v in C(u)
	for (vector<int>::iterator v = (&candidates_u)->begin(); v != (&candidates_u)->end(); v++) {
		if (needReturn) {
			return;
		}
		if ((*numberOfEmbeddings)[queryGraph->graphId] >= GlobalConstant::G_SUFFICIENT_NUMBER_EMBEDDINGS) {
			needReturn = true;
			break; // only calculate 1000 embeddings for each query graph
		}
		//refine candidate
		if (!refineCandidates(u, *v)) {
			continue;
		}
		
		if (isJoinable(u.id, *v)) {
			updateState(u.id, *v);
			subgraphSearch();
			restoreState(u.id, *v);
		}
	}

}

AdjacenceListsGRAPH::Vertex EmBoost::nextQueryVertex() {
	for (int i = 0; i<queryGraph->getNumberOfVertexes(); i++) {
		if (embedding[queryGraph->getDFSTraversalOrder()->at(i)] == -1) {
			return queryGraph->getVertexList()->at(queryGraph->getDFSTraversalOrder()->at(i));
		}
	}
}

bool EmBoost::isJoinable(int u, int v) {

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

bool EmBoost::refineCandidates(AdjacenceListsGRAPH::Vertex & u, const int v) {
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
bool EmBoost::degreeFilter(int u, int v) {
	// TODO, no need extra index
	//return true;
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

void EmBoost::showEmbedding() {
	std::cout << "{";
	for (int i = 0; i < queryGraph->getNumberOfVertexes(); i++) {
		cout << "*: " << i << "->" << embedding[i] << " , ";
	}
	cout << "}" << endl;
}