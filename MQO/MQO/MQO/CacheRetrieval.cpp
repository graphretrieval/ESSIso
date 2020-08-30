#include "CacheRetrieval.h"


using namespace std;

CacheRetrieval::CacheRetrieval(AdjacenceListsGRAPH * pDataGraph, std::vector<std::vector<CacheEmbeddingNode *>> * pResultCaches)
{
	dataGraph = pDataGraph;
	resultCaches = pResultCaches;
}

void CacheRetrieval::retrieveEmbeddingCandidate(AdjacenceListsGRAPH * pQueryGraph, AdjacenceListsGRAPH * pParentGraph, JointGraphNode * pJointGraphNode, JointNodeOrderNode * pJointNodeOrderNode, int * pEmbedding, std::map<int, int> * pInverseEmbedding, std::vector<std::vector<CacheEmbeddingNode *>> * pCandidates)
{
	queryGraph = pQueryGraph;
	parentGraph = pParentGraph;
	embedding = pEmbedding;
	inverseEmbedding = pInverseEmbedding;
	candidates = pCandidates;
	jointGraphNode = pJointGraphNode;

	// we need this mapping to power the isJoinable
	jointNodeOrderNode = pJointNodeOrderNode;

	comboVertexMapping = new CacheEmbeddingNode*[parentGraph->getComboGraph()->size()];
	for (int i = 0; i < parentGraph->getComboGraph()->size(); i++) {
		comboVertexMapping[i] = NULL;
	}

	/*
	 * enumerate the embeddings list according to a DFS order
	 */
	int comboNodeId = parentGraph->getDFSComboVertexOrder()->at(0);
	ComboNODE & comboNode = parentGraph->getComboGraph()->at(comboNodeId);
	CacheEmbeddingNode * comboVertexStarting = (*resultCaches)[parentGraph->graphId][comboNodeId];

	while (comboVertexStarting != NULL) {
		if (isJoinable(comboVertexStarting, &comboNode)){
			comboVertexMapping[parentGraph->getDFSComboVertexOrder()->at(0)] = comboVertexStarting;
			recursiveEnumLinkedLists(1);
		}
		comboVertexStarting = comboVertexStarting->adj;
	}

	delete[] comboVertexMapping;
}



void CacheRetrieval::recursiveEnumLinkedLists(int comboVertexOrder) {

	if (comboVertexOrder == parentGraph->getDFSComboVertexOrder()->size()) {
		// Found a complete candidate add it to the list
		candidates->push_back(vector<CacheEmbeddingNode*>());
		vector<CacheEmbeddingNode *> & newCandidate = candidates->at(candidates->size() - 1);
		for (int comboNodeId = 0; comboNodeId < parentGraph->getComboGraph()->size(); comboNodeId++) {
			newCandidate.push_back(comboVertexMapping[comboNodeId]);
		}
		return;
	}

	int comboVertexId = parentGraph->getDFSComboVertexOrder()->at(comboVertexOrder);
	ComboNODE & comboNode = parentGraph->getComboGraph()->at(comboVertexId);

	std::map<int, std::set<CacheEmbeddingNode *>>::iterator comboMappingIterator = comboVertexMapping[comboNode.parent]->neighboursList.find(comboVertexId);

	for (std::set<CacheEmbeddingNode *>::iterator comboMapping = comboMappingIterator->second.begin(); comboMapping != comboMappingIterator->second.end(); comboMapping++) {
		bool canBeAdded = true;
		/*
		 * for each of combo embedding explored by one of its parent combo embedding.
		 */
		for (std::set<int>::iterator comboVertexNeiIterator = comboNode.neighbours.begin(); comboVertexNeiIterator != comboNode.neighbours.end(); comboVertexNeiIterator++) {
			/*
			 * for each of non-spanning tree edges here. we need to check the connectivity.
			 */
			if (*comboVertexNeiIterator != comboNode.parent) {
				if (comboVertexMapping[*comboVertexNeiIterator] != NULL) {
					if (*comboVertexNeiIterator < comboVertexId) {
						std::map<int, std::set<CacheEmbeddingNode *>>::iterator nonTreeMappingIter = comboVertexMapping[*comboVertexNeiIterator]->neighboursList.find(comboVertexId);
						if (nonTreeMappingIter->second.find(*comboMapping) == nonTreeMappingIter->second.end()) {
							canBeAdded = false;
							break;
						}
					}
					else {
						std::map<int, std::set<CacheEmbeddingNode *>>::iterator nonTreeMappingIter = (*comboMapping)->neighboursList.find(*comboVertexNeiIterator);
						if (nonTreeMappingIter->second.find(comboVertexMapping[*comboVertexNeiIterator]) == nonTreeMappingIter->second.end()) {
							canBeAdded = false;
							break;
						}
					}
				}
			}
		}
		if (canBeAdded || !isJoinable(*comboMapping, &comboNode)) {
			continue;
		}
		comboVertexMapping[comboVertexId] = *comboMapping;
		recursiveEnumLinkedLists(comboVertexOrder + 1);
		comboVertexMapping[comboVertexId] = NULL;
	}

}


bool CacheRetrieval::isJoinable(CacheEmbeddingNode * embeddingNode, ComboNODE * comboNode) {

	for (int queryVertexIndex = 0; queryVertexIndex < comboNode->queryVertices.size(); queryVertexIndex++) {

		int parentQueryVertexId = comboNode->queryVertices[queryVertexIndex];
		int dataVertexId = embeddingNode->comboMappingVertices[queryVertexIndex];


		if (jointNodeOrderNode->preMultiCoveredVertex[parentQueryVertexId] != -1) {
			if (dataVertexId != embedding[jointNodeOrderNode->preMultiCoveredVertex[parentQueryVertexId]]) {
				return false;
			}
		}

		for (std::map<int, vector<int>>::iterator uncoveIter = jointNodeOrderNode->uncoveredEdges[parentQueryVertexId].begin(); uncoveIter != jointNodeOrderNode->uncoveredEdges[parentQueryVertexId].end(); uncoveIter++) {
			if (comboVertexMapping[uncoveIter->first] != NULL) {
				
				ComboNODE & comboNode = parentGraph->getComboGraph()->at(uncoveIter->first);
				
				for (vector<int>::iterator uncoQurVer = uncoveIter->second.begin(); uncoQurVer != uncoveIter->second.end(); uncoQurVer++) {
					for (int comboVerIter = 0; comboVerIter < comboNode.queryVertices.size(); comboVerIter++) {\
						if (jointGraphNode->mapping[comboNode.queryVertices[comboVerIter]] == *uncoQurVer) {
							
							if (!dataGraph->edge(dataVertexId, comboVertexMapping[uncoveIter->first]->comboMappingVertices[comboVerIter])) {
								return false;
							}
						}
					}
				}
			}
		}
	}

	return true;
}


/* u is the query vertex id, v is the data graph vertex id */
bool CacheRetrieval::degreeFilter(int u, int v) {

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
