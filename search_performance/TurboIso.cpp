#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <queue>
#include <chrono>
#include <limits> 

#include "Graph.hpp"
#include "TurboIso.hpp"

std::vector<std::map<int,int>> TurboIso::getAllSubGraphs(const Graph & pdataGraph, const Graph &pqueryGraph, int maxRCall_, int maxEmbedding_) {
    startTime = std::chrono::steady_clock::now();
    dataGraph = pdataGraph;
    queryGraph = pqueryGraph;
	maxEmbedding = maxEmbedding_;
	maxRCall = maxRCall_;
	nCall = 0;
	needReturn = false;
	allMappings.clear();
	mapping.clear();
	largestMapping.clear();
	reverseMapping.clear();
    necTree.clear();
    queryMatchingSuquence.clear();

    startQueryVertex = chooseStartVertex();
    if (startQueryVertex == -1) {
		return allMappings;
	}

	rewriteToNecTree();
    std::set<int> startVertexCandidates = dataGraph.labelVertexList.find(queryGraph.label[startQueryVertex])->second;	
    for (std::set<int>::iterator startVertexCandidateIterator = startVertexCandidates.begin(); startVertexCandidateIterator != startVertexCandidates.end(); startVertexCandidateIterator++) {

        std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();

        if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() >= timeOut*1000) 
            throw std::runtime_error("Timeout");
        
        if ((nCall >= maxRCall) && maxRCall>0)  {
            needReturn = true;
            return allMappings;
        }   

		if (needReturn) {
			break;
		}

        if (allMappings.size() >= maxEmbedding) {
            needReturn = true;
            return allMappings;
        }

		std::vector<int> subRegionCandidates;
		subRegionCandidates.push_back(*startVertexCandidateIterator);

		// explore the ER start from the root of the nectree
		CR = std::map<std::pair<int, int>, std::set<int>>();

		if (exploreCR(0, -1, subRegionCandidates) == false) {
			continue;
		}

	// 	/*
	// 	* Computing the vertication order based on the saved CR
	// 	*/
		computeMatchingOrder();

		updateState(startQueryVertex, *startVertexCandidateIterator);
		subgraphSearch();
		restoreState(startQueryVertex, *startVertexCandidateIterator);
	} 
	return allMappings;
}

int TurboIso::chooseStartVertex() {

	std::vector<float> rankScore;
    for (int u=0; u < queryGraph.n; u ++) {
        if (queryGraph.adjList[u].size() != 0) {
            std::map<int, std::set<int>>::iterator labelVertexList = queryGraph.labelVertexList.find(queryGraph.label[u]);
            if (labelVertexList != queryGraph.labelVertexList.end()) {
                rankScore.push_back((float)labelVertexList->second.size()/ queryGraph.adjList[u].size());
            } else {
                return -1;
            }
        }
    }
	// minimum query vertex number is 2
	int topThree[3];
	for (int i = 0; i<3; i++) {
		topThree[i] = -1;
	}
	float miniScore = std::numeric_limits<float>::max();
	for (size_t i = 0; i<rankScore.size(); i++) {
		if (rankScore[i] < miniScore) {
			miniScore = rankScore[i];
			topThree[0] = i;
		}
	}
	miniScore =std::numeric_limits<float>::max();
	for (size_t i = 0; i<rankScore.size(); i++) {
		if (i != topThree[0]) {
			if (rankScore[i] < miniScore) {
				miniScore = rankScore[i];
				topThree[1] = i;
			}
		}
	}
	miniScore =std::numeric_limits<float>::max();
	if (rankScore.size() >= 3) {
		for (size_t i = 0; i<rankScore.size(); i++) {
			if (i != topThree[0] && i != topThree[1]) {
				if (rankScore[i] < miniScore) {
					miniScore = rankScore[i];
					topThree[2] = i;
				}
			}
		}
	}
	int topThreeNumberOfCandidates[3];
	int us;
	int miniTopThree = INT_MAX;
	for (int i = 0; i<3; i++) {
		topThreeNumberOfCandidates[i] = 0;
		if (topThree[i] != -1) {
			std::map<int, std::set<int>>::iterator candidateListIterator = dataGraph.labelVertexList.find(queryGraph.label[topThree[i]]);
			if (candidateListIterator == dataGraph.labelVertexList.end()) {
				// the query vertex doesn't have candidates
				return -1;
			}
			for (std::set<int>::iterator candidateIterator = candidateListIterator->second.begin();
			candidateIterator != candidateListIterator->second.end(); candidateIterator++) {
				if (degreeFilter(topThree[i], *candidateIterator)) {
					topThreeNumberOfCandidates[i] ++;
				}
			}
			if (miniTopThree > topThreeNumberOfCandidates[i]) {
				us = topThree[i];
				miniTopThree = topThreeNumberOfCandidates[i];
			}
		}
	}
	return us;
}

bool TurboIso::degreeFilter(int u, int v) {
	for (std::map<int, std::vector<int>>::iterator queryVertexLabelVertexIt = queryGraph.vertexlabelVertexList[u].begin(); 
		queryVertexLabelVertexIt != queryGraph.vertexlabelVertexList[u].end(); queryVertexLabelVertexIt++ ) {
		if (dataGraph.vertexlabelVertexList[v].find(queryVertexLabelVertexIt->first) ==dataGraph.vertexlabelVertexList[v].end() )
			return false;
		else if (queryVertexLabelVertexIt->second.size() > dataGraph.vertexlabelVertexList[v].find(queryVertexLabelVertexIt->first)->second.size())
			return false;
	}
	return true;
}

void TurboIso::rewriteToNecTree() {
	bool *flags = new bool[queryGraph.n];
	for (int i = 0; i<queryGraph.n; i++) {
		flags[i] = false;
	}

	std::queue<std::pair<int, int>> S; // id, parentId
	int vertexId, parentId;

	S.push(std::pair<int, int>(startQueryVertex, -1));
	flags[startQueryVertex] = true;

	while (!S.empty()) {
		vertexId = S.front().first;
		parentId = S.front().second;
		S.pop();
		necTree.push_back(NECNode());
		NECNode & necNode = *(necTree.rbegin());
		necNode.vertexId = vertexId;
		necNode.parent = parentId;
		necNode.id = necTree.size() - 1;
		necNode.label = queryGraph.label[vertexId];

		if (parentId != -1)
			necTree[parentId].childList.push_back(necNode.id);

        for (std::set<int>::iterator it = queryGraph.adjList[vertexId].begin(); it != queryGraph.adjList[vertexId].end(); it++) {
            if (!flags[*it]) {
                flags[*it] = true; 
                S.push(std::pair<int,int>(*it, necNode.id));
            }
        }
	}
	delete[] flags;
}


bool TurboIso::exploreCR(int u, int parentMappedNecTree, std::vector<int> & subRegionCandidates) {
	
	/*
	if (CRRecursiveCallsNo++ >= GlobalConstant::G_exploreCRRecursiveCalls) {
		needReturn = true;
		return false;
	}*/

	for (size_t vmIndex = 0; vmIndex < subRegionCandidates.size(); vmIndex++) {

		bool matched = true;

		for (std::vector<int>::iterator childIterator = necTree[u].childList.begin(); childIterator != necTree[u].childList.end(); childIterator++) {

			std::vector<int> adjChildAdj;

			// AdjacenceListsGRAPH::Vertex dataGraphChild = dataGraph->getVertexByVertexId(subRegionCandidates[vmIndex]);
            int dataGraphChild = subRegionCandidates[vmIndex];
			// map<int, vector<int>>::iterator childLabelList = dataGraphChild.labelVertexList.find(necTree[*childIterator].label);
			std::map<int, std::vector<int>>::iterator childLabelList = dataGraph.vertexlabelVertexList[dataGraphChild].find(necTree[*childIterator].label);

			if (childLabelList != dataGraph.vertexlabelVertexList[dataGraphChild].end()) {
				for (std::vector<int>::iterator childLabelItem = childLabelList->second.begin(); childLabelItem != childLabelList->second.end(); childLabelItem++) {

					if (!degreeFilter(necTree[*childIterator].vertexId, *childLabelItem)) {
						continue;
					}

					adjChildAdj.push_back(*childLabelItem);
				}
			}


			/*
			* A important modification for the exploreCR for BoostIso goes here. We need to add itself here to suit for the dynamic candidate loading strategy
			*/
			if (parentMappedNecTree == -1) {
				if (degreeFilter(necTree[*childIterator].vertexId, subRegionCandidates[vmIndex])) {
					adjChildAdj.push_back(subRegionCandidates[vmIndex]);
				}
			}

			if (exploreCR(necTree[*childIterator].id, subRegionCandidates[vmIndex], adjChildAdj) == false) {
				/*
				* If one of its child cannot match under this subregion
				* 1. stop the process, and the elements at "vmIndex" won't be added
				* Two reasons not to delete anything.
				* 1. No need to, we can allow some rubbish data in the memory while no need to bother to clean them before the computing of this query is done.
				* 2. In some special cases, you cannot delete it. A grand-relationship will cause error.
				*/
				matched = false;
				break;
			}
		}
		if (matched == false) {
			continue;
		}
		std::map<std::pair<int, int>, std::set<int>>::iterator  CRIterator = CR.find(std::pair<int, int>(u, parentMappedNecTree));
		if (CRIterator == CR.end()) {
			std::set<int> CRVertices;
			CRVertices.insert(subRegionCandidates[vmIndex]);
			CR.insert(std::pair<std::pair<int, int>, std::set<int>>(std::pair<int, int>(u, parentMappedNecTree), CRVertices));
		}
		else {
			CRIterator->second.insert(subRegionCandidates[vmIndex]);
		}
	}

	if (CR.find(std::pair<int, int>(u, parentMappedNecTree)) == CR.end()) {
		return false;
	}

	return true;
}


void TurboIso::computeMatchingOrder() {

	// set the start vertex
	float * queryVertexScore = new float[necTree.size()];
	for (size_t queryVertexIndex = 0; queryVertexIndex<necTree.size(); queryVertexIndex++) {
		queryVertexScore[queryVertexIndex] = 0;
	}

	for (std::map<std::pair<int, int>, std::set<int>>::iterator crIterator = CR.begin(); crIterator != CR.end(); crIterator++) {
		queryVertexScore[crIterator->first.first] += crIterator->second.size();
	}

	for (size_t queryVertexIndex = 0; queryVertexIndex<necTree.size(); queryVertexIndex++) {
		// devide the no-tree edges
		int numberOfNoTreeEdges;
		if (necTree[queryVertexIndex].parent != -1) {
			numberOfNoTreeEdges = queryGraph.adjList[necTree[queryVertexIndex].vertexId].size() - necTree[queryVertexIndex].childList.size() - 1;
		}
		else {
			numberOfNoTreeEdges = queryGraph.adjList[necTree[queryVertexIndex].vertexId].size() - necTree[queryVertexIndex].childList.size();
		}
		devideNoTreeEdges(queryVertexIndex, numberOfNoTreeEdges + 1, queryVertexScore);
	}

	std::vector<int> nextVertex;
	for (std::vector<int>::iterator childIterator = necTree[0].childList.begin(); childIterator != necTree[0].childList.end(); childIterator++) {
		nextVertex.push_back(*childIterator);
	}
	getOrderByBFSScore(0, queryVertexScore, nextVertex);


	delete[] queryVertexScore;
}

void TurboIso::devideNoTreeEdges(int u, int numberOfNoTreeEdges, float * queryVertexScore) {
	queryVertexScore[u] /= numberOfNoTreeEdges;
	for (std::vector<int>::iterator childIterator = necTree[u].childList.begin(); childIterator != necTree[u].childList.end(); childIterator++) {
		devideNoTreeEdges(*childIterator, numberOfNoTreeEdges, queryVertexScore);
	}
}

void TurboIso::getOrderByBFSScore(int u, float * queryVertexScore, std::vector<int> & nextVertex) {

	queryMatchingSuquence.push_back(u);

	if (nextVertex.size() == 0) {
		return;
	}

	float mimimum = std::numeric_limits<float>::max();;
	int mimimumVertex = 1;
	int vertexNextId = -1;

	for (size_t i = 0; i < nextVertex.size(); i++) {
		if (queryVertexScore[nextVertex[i]] == -1) {
			continue;
		}
		else {
			if (queryVertexScore[nextVertex[i]] < mimimum) {
				mimimumVertex = nextVertex[i];
				vertexNextId = i;
				mimimum = queryVertexScore[nextVertex[i]];
			}
		}
	}
	queryVertexScore[mimimumVertex] = -1;
	if (vertexNextId != -1) {
		nextVertex.erase(nextVertex.begin() + vertexNextId);
	}


	for (std::vector<int>::iterator childIterator = necTree[mimimumVertex].childList.begin(); childIterator != necTree[mimimumVertex].childList.end(); childIterator++) {
		nextVertex.push_back(*childIterator);
	}
	getOrderByBFSScore(mimimumVertex, queryVertexScore, nextVertex);
}

void TurboIso::subgraphSearch() {

	std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();

	if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() >= timeOut*1000) 
		throw std::runtime_error("Timeout");

	nCall++;
	if ((nCall >= maxRCall) && maxRCall>0)  {
		needReturn = true;
		return;
	}
	
	if (needReturn)
		return;
	if (mapping.size() == queryGraph.n) {
		std::map<int,int> tempMapping = mapping;
		allMappings.push_back(tempMapping);
		if (allMappings.size() >= maxEmbedding)
			needReturn = true;
		return;
	}

	NECNode  * nectree_u = nextQueryVertex();
	std::map<std::pair<int, int>, std::set<int>>::iterator candidateListIterator = CR.find(std::pair<int, int>(nectree_u->id, mapping[necTree[nectree_u->parent].vertexId]));
	if (candidateListIterator == CR.end()) {
		return;
	}

	for (std::set<int>::iterator candidateIterator = candidateListIterator->second.begin(); candidateIterator != candidateListIterator->second.end(); candidateIterator++) {
		if (needReturn) {
			return;
		}

		// For the completeness of MQO, we have to let the algorithm run until find all the embeddings. 
		
		if (allMappings.size() >= maxEmbedding) {
			needReturn = true;
			break; // only calculate 1000 embeddings for each query graph
		}

		// all ready match filter 
		if (reverseMapping.find(*candidateIterator) != reverseMapping.end()) {
			continue;
		}
		// isJoinable Filtering
		if (!isJoinable(nectree_u->vertexId, *candidateIterator)) {
			continue;
		}
		updateState(nectree_u->vertexId, *candidateIterator);
		subgraphSearch();
		restoreState(nectree_u->vertexId, *candidateIterator);
	}	
}

TurboIso::NECNode * TurboIso::nextQueryVertex()
{
	return &necTree[queryMatchingSuquence[reverseMapping.size()]];
}


bool TurboIso::isJoinable(int u, int v) {
	for (std::set<int>::iterator neighNodeU = queryGraph.adjList[u].begin(); neighNodeU != queryGraph.adjList[u].end(); neighNodeU++) {
		std::map<int,int>::iterator it = mapping.find(*neighNodeU);
		if (it == mapping.end()) {
			continue;
		} else {
			// if (dataGraph.adjmat[v][it->second]==1)
			if (dataGraph.adjList[v].find(it->second)!=dataGraph.adjList[v].end())
				continue;
			else
				return false;
		}
	}
	return true;
}

void TurboIso::updateState(int u, int v) {
	mapping.insert({u, v});
	if (mapping.size() > largestMapping.size())
		largestMapping = mapping;
	reverseMapping.insert({v,u});
}

void TurboIso::restoreState(int u, int v) {
	mapping.erase(u);
	reverseMapping.erase(v);
}