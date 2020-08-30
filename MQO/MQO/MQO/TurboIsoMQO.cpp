#include"TurboIsoMQO.h"
#include<queue>
#include<map>
#include<algorithm>
#include"CacheEmbeddingNode.h"
#include"ComboLinkedLists.h"
#include"CacheRetrieval.h"
#include"GlobalConstant.h"
#include <climits>

TurboIsoMQO::TurboIsoMQO(AdjacenceListsGRAPH * pDataGraph, std::vector<std::vector<CacheEmbeddingNode *>> * pResultCaches, std::vector<int> * pNumberOfEmbeddings,
	std::vector<AdjacenceListsGRAPH> * pQueryGraphVector, std::vector<AdjacenceListsGRAPH> * pNewlyGeneratedGraphVector) {
	
	dataGraph = pDataGraph;

	numberOfEmbeddings = pNumberOfEmbeddings;
	resultCaches = pResultCaches;

	queryGraphVector = pQueryGraphVector;
	newlyGeneratedGraphVector = pNewlyGeneratedGraphVector;

}

TurboIsoMQO::~TurboIsoMQO() {
	
	jointNodeMatchingOrder = std::vector<JointNodeOrderNode>();
	inverseEmbedding = std::map<int, int>();
}

void TurboIsoMQO::setParameters(AdjacenceListsGRAPH * pQueryGraph, JointGraph * pJointGraph, bool pNeedSaveCache) {
	
	queryGraph = pQueryGraph;
	jointGraph = pJointGraph;
	needSaveCache = pNeedSaveCache;
	
	needReturn = false;
	recursiveCallsNo = 0;

	
	embedding = new int[queryGraph->getNumberOfVertexes()];
	for (int i = 0; i < queryGraph->getNumberOfVertexes(); i++) {
		embedding[i] = -1;
	}

	jointNodeMatchingOrder.clear();
	inverseEmbedding = std::map<int, int>();

}

void TurboIsoMQO::execute(){
	/*
	 * Compute the joint node matching order. each joint node can represent a parent or a stand along vertex
	 */
	startTime = std::chrono::steady_clock::now();
	computeMatchingOrder();
	
	if (jointNodeMatchingOrder.size() != jointGraph->jointNodes.size()) {
		/*
		 * some joint node doesn't have any candidates
		 */
		return;
	}

	/*
	 * add the candidate according to the pre-computed order
	 */
	resuriveSearch(0);
}

void TurboIsoMQO::resuriveSearch(int matchingOrderIndex) {
	std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
	if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() >= GlobalConstant::timeOut) 
		throw std::runtime_error("Timeout");
	if (recursiveCallsNo++ >= GlobalConstant::G_enoughRecursiveCalls && GlobalConstant::G_enoughRecursiveCalls > 0) {
		// (*numberOfEmbeddings)[queryGraph->graphId] = -1;
		needReturn = true;
		return;
	}
	if ((*numberOfEmbeddings)[queryGraph->graphId] >= GlobalConstant::G_SUFFICIENT_NUMBER_EMBEDDINGS) {
		needReturn=true;
		return;
	}
	if (matchingOrderIndex == jointNodeMatchingOrder.size()) {

#ifdef _DEBUG
		cout << "Find a Embedding for query " << queryGraph->graphId << "{";
		for (int i = 0; i < queryGraph->getNumberOfVertexes(); i++) {
			cout << embedding[i] << " ";
		}
		cout << " } " << endl;
#endif

		(*numberOfEmbeddings)[queryGraph->graphId] ++;
		// already reached to the last node. a full embedding was found
		if (needSaveCache) {
			ComboLinkedLists::addEmbeddingToCache(resultCaches, queryGraph, embedding);
		}
	}
	else {
		JointGraphNode & jointNode = jointGraph->jointNodes[jointNodeMatchingOrder[matchingOrderIndex].jointNodeId];

		if (jointNode.parentId != -1) {
			/*
			 * the next joint node represents a parent graph, we enumerate the candaite and for each candidate, we try to go to next iteration
			 */
			AdjacenceListsGRAPH * parentGraph = NULL;
			if (jointNode.parentId < queryGraphVector->size()) {
				parentGraph = &(*queryGraphVector)[jointNode.parentId];
			}
			else {
				parentGraph = &(*newlyGeneratedGraphVector)[jointNode.parentId - queryGraphVector->size()];
			}

			/*
			 * enumerate the embeddings from the cache
			 */
			CacheRetrieval * cacheRetrieval = new CacheRetrieval(dataGraph, resultCaches);
			std::vector<std::vector<CacheEmbeddingNode *>> candidates;
			cacheRetrieval->retrieveEmbeddingCandidate(queryGraph, parentGraph, &jointNode, &jointNodeMatchingOrder[matchingOrderIndex], embedding, &inverseEmbedding, &candidates);

#ifdef _DEBUG
			cout << "Candidate Size: " << candidates.size() << endl;
#endif

			/*
			 * will be updated query vertex
			 */
			vector<int> updatingQueryVertex;
			for (int comboNodeId = 0; comboNodeId < parentGraph->getComboGraph()->size(); comboNodeId++) {
				ComboNODE & comboNode = (*(parentGraph->getComboGraph()))[comboNodeId];
				for (int comboVertexIter = 0; comboVertexIter < comboNode.queryVertices.size(); comboVertexIter++) {
					int queryVertexId = jointNode.mapping[comboNode.queryVertices[comboVertexIter]];
					if (embedding[queryVertexId] == -1) {
						updatingQueryVertex.push_back(queryVertexId);
					}
				}
			}


			for (std::vector<std::vector<CacheEmbeddingNode *>>::iterator candidateIter = candidates.begin(); candidateIter != candidates.end(); candidateIter++) {
				if (needReturn) {
					return;
				}		
				for (int comboNodeId = 0; comboNodeId < parentGraph->getComboGraph()->size(); comboNodeId++) {
					// each combo node corresponding to cache embedding node which contains multiple matched data vertices
					ComboNODE & comboNode = (*(parentGraph->getComboGraph()))[comboNodeId];
					for (int comboVertexIter = 0; comboVertexIter < comboNode.queryVertices.size(); comboVertexIter++) {

						int dataVertexId = (*candidateIter)[comboNodeId]->comboMappingVertices[comboVertexIter];
						int queryVertexId = jointNode.mapping[comboNode.queryVertices[comboVertexIter]];
						if (embedding[queryVertexId] == -1) {
							updateState(queryVertexId, dataVertexId);
						}
					}
				}
				// next iteration
				resuriveSearch(matchingOrderIndex + 1);

				// recover the last
				for (vector<int>::iterator updatedVIter = updatingQueryVertex.begin(); updatedVIter != updatingQueryVertex.end(); updatedVIter++) {
					restoreState(*updatedVIter);
				}
			}
			delete cacheRetrieval;
		}
		else {
			/* 
			 * the next node represents a stand alone query vertex.
			 * stand alone query vertex cannot be matched before. so no need to ensure multi-map consistency
			 */
			AdjacenceListsGRAPH::Vertex * queryVertex = queryGraph->getVertexAddressByVertexId(jointNode.mapping[0]);
			std::vector<int> * candidates = NULL;

			if (matchingOrderIndex == 0) {
				// the starting vertex cannot use exploration
				std::map<int, std::vector<int>>::iterator candidateIter = dataGraph->getLabelVertexList()->find(queryVertex->label);
				if (candidateIter == dataGraph->getLabelVertexList()->end()) {
					return;
				}

				candidates = &candidateIter->second;
			}
			else {
				/*
				 * the following vertex use candidate exploration from its mapped parents.
				 * we explore a connected, already mapped query vertex which generate minimum number of candidates.
				 */
				int minCandidateSize = INT_MAX;
				for (size_t queryVertexIndex = 0; queryVertexIndex <= queryGraph->getNumberOfVertexes(); queryVertexIndex++) {
					if (embedding[queryVertexIndex] != -1 && queryGraph->edge(queryVertex->id, queryVertexIndex)) {
						AdjacenceListsGRAPH::Vertex * alreadyMappedDataVer = dataGraph->getVertexAddressByVertexId(embedding[queryVertexIndex]);
						std::map<int, std::vector<int>>::iterator candidateIter = alreadyMappedDataVer->labelVertexList.find(queryVertex->label);
						if (candidateIter == alreadyMappedDataVer->labelVertexList.end()) {
							return;
						}
						else {
							if (candidateIter->second.size() < minCandidateSize) {
								minCandidateSize = candidateIter->second.size();
								candidates = &candidateIter->second;
							}
						}
					}
				}
			}
			/*
			 * for each of the candidates, we further iterate
			 */
			for (vector<int>::iterator candidateIter = candidates->begin(); candidateIter != candidates->end(); candidateIter++) {
				if (needReturn) {
					return;
				}
				if (degreeFilter(queryVertex->id, *candidateIter) && isJoinable(queryVertex->id, *candidateIter)) {
					updateState(queryVertex->id, *candidateIter);
					resuriveSearch(matchingOrderIndex + 1);
					restoreState(queryVertex->id);
				}
			}
		}
	}
}

/*
 * Given a PCM. we pre-compute a matching order here. 
 * to use the exploration similar to turboIso. we can define a BFS order
 */
void TurboIsoMQO::computeMatchingOrder() {
	
	//build candidate size map
	vector<std::pair<int, int>> inverseMap;

	for (size_t nodeIndex = 0; nodeIndex < jointGraph->jointNodes.size(); nodeIndex++) {
		int parentId = jointGraph->jointNodes[nodeIndex].parentId;
		if (parentId != -1) {
			inverseMap.push_back(std::pair<int, int>((*numberOfEmbeddings)[parentId], nodeIndex));
		}
		else {
			AdjacenceListsGRAPH::Vertex * queryVertex = queryGraph->getVertexAddressByVertexId(jointGraph->jointNodes[nodeIndex].mapping[0]);
			std::map<int, std::vector<int>>::iterator candidates = dataGraph->getLabelVertexList()->find(queryVertex->label);
			if (candidates != dataGraph->getLabelVertexList()->end()) {
				inverseMap.push_back(std::pair<int, int>(candidates->second.size(), nodeIndex));
			}
			else {
				return;
			}
		}
	}
	std::sort(inverseMap.begin(), inverseMap.end());
	

	// bfs order, start from the minimum size cache
	bool * flags = new bool[jointGraph->jointNodes.size()];
	for (int i = 0; i < jointGraph->jointNodes.size(); i++) {
		flags[i] = false;
	}
	queue<int> visited;


	JointNodeOrderNode startNodeOrderNode;
	//@debug
	/*for (size_t i = 0; i < jointGraph->jointNodes.size(); i++) {
		if (jointGraph->jointNodes[i].parentId != -1) {
			visited.push(i);
			flags[i] = true;
			startNodeOrderNode.jointNodeId = i;
			break;
		}
	}*/
	
	startNodeOrderNode.jointNodeId = inverseMap[0].second;
	visited.push(inverseMap[0].second);
	flags[inverseMap[0].second] = true;
	jointNodeMatchingOrder.push_back(startNodeOrderNode);

	while (!visited.empty()) {
		int nodeId = visited.front();
		visited.pop();

		for (size_t nodeIndex = 0; nodeIndex < jointGraph->jointNodes.size(); nodeIndex++) {
			if (!flags[nodeIndex] && jointGraph->edge(nodeIndex, nodeId)) {

				flags[nodeIndex] = true;
				visited.push(nodeIndex);
				
				JointNodeOrderNode nextJointNodeOrderNode;
				nextJointNodeOrderNode.jointNodeId = nodeIndex;
				jointNodeMatchingOrder.push_back(nextJointNodeOrderNode);
			}
		}
	}
	delete[] flags;


	/*
	 * add the common or connected already mapped query vertices
	 */
	for (int i = 0; i < jointNodeMatchingOrder.size(); i++) {

		JointNodeOrderNode & i_jointNodeOrderNode = jointNodeMatchingOrder[i];
		JointGraphNode & i_jointGraphNode = jointGraph->jointNodes[i_jointNodeOrderNode.jointNodeId];

		
		/*
		 * initialize it
		 */
		i_jointNodeOrderNode.preMultiCoveredVertex = vector<int>(i_jointGraphNode.mapping.size());
		for (int mappingIdex = 0; mappingIdex < i_jointGraphNode.mapping.size(); mappingIdex++) {
			i_jointNodeOrderNode.preConnectedMappedVertex.push_back(vector<int>());
			i_jointNodeOrderNode.preMultiCoveredVertex[mappingIdex] = -1;

			i_jointNodeOrderNode.uncoveredEdges.push_back(std::map<int, vector<int>>());
		}

		for (int j = 0; j < i; j++) {

			JointNodeOrderNode & j_jointNodeOrderNode = jointNodeMatchingOrder[j];
			JointGraphNode & j_jointGraphNode = jointGraph->jointNodes[j_jointNodeOrderNode.jointNodeId];

			if (jointGraph->edge(i_jointNodeOrderNode.jointNodeId, j_jointNodeOrderNode.jointNodeId)) {

				// if these two joint node has edges
				for (int i_mappingIdex = 0; i_mappingIdex < i_jointGraphNode.mapping.size(); i_mappingIdex++) {
					// for each of the vertex within this joint node
					for (int j_mappingIdex = 0; j_mappingIdex < j_jointGraphNode.mapping.size(); j_mappingIdex++) {
						if (i_jointGraphNode.mapping[i_mappingIdex] == j_jointGraphNode.mapping[j_mappingIdex]) {
							// this two joint node covered a common query vertex

							i_jointNodeOrderNode.preMultiCoveredVertex[i_mappingIdex] = j_jointGraphNode.mapping[j_mappingIdex];
							if (i_jointNodeOrderNode.preConnectedMappedVertex[i_mappingIdex].size() > 0) {
								i_jointNodeOrderNode.preConnectedMappedVertex[i_mappingIdex] = vector<int>();
							}
							break;
						}
						else if (queryGraph->edge(i_jointGraphNode.mapping[i_mappingIdex], j_jointGraphNode.mapping[j_mappingIdex])) {
							i_jointNodeOrderNode.preConnectedMappedVertex[i_mappingIdex].push_back(j_jointGraphNode.mapping[j_mappingIdex]);
						}
					}
				}
			}
		}
	}
	/*
	 * add uncovered query vertices
	 */
	for (int i = 0; i < jointNodeMatchingOrder.size(); i++) {

		JointNodeOrderNode & jointNodeOrderNode = jointNodeMatchingOrder[i];
		JointGraphNode & jointGraphNode = jointGraph->jointNodes[jointNodeOrderNode.jointNodeId];

		if (jointGraphNode.parentId == -1) {
			continue;
		}

		AdjacenceListsGRAPH * parentGraph = NULL;
		if (jointGraphNode.parentId >= queryGraphVector->size()) {
			parentGraph = &(*newlyGeneratedGraphVector)[jointGraphNode.parentId - queryGraphVector->size()];
		}
		else {
			parentGraph = &(*queryGraphVector)[jointGraphNode.parentId];
		}
		std::vector<ComboNODE> * comboGraph = parentGraph->getComboGraph();
		for (int i_comboIndex = 0; i_comboIndex < comboGraph->size(); i_comboIndex++) {
			for (int j_comboIndex = i_comboIndex + 1; j_comboIndex < comboGraph->size(); j_comboIndex++) {
				

				for (std::vector<int>::iterator i_comboVerIter = (*comboGraph)[i_comboIndex].queryVertices.begin(); i_comboVerIter != (*comboGraph)[i_comboIndex].queryVertices.end(); i_comboVerIter++) {
					for (std::vector<int>::iterator j_comboVerIter = (*comboGraph)[j_comboIndex].queryVertices.begin(); j_comboVerIter != (*comboGraph)[j_comboIndex].queryVertices.end(); j_comboVerIter++) {
						
						if (!parentGraph->edge(*i_comboVerIter, *j_comboVerIter)) {
							if (queryGraph->edge(jointGraphNode.mapping[*i_comboVerIter], jointGraphNode.mapping[*j_comboVerIter])) {
								// this is a uncovered edge
								std::map<int, vector<int>>::iterator uncoverIter = jointNodeOrderNode.uncoveredEdges[*i_comboVerIter].find(j_comboIndex);
								if (uncoverIter == jointNodeOrderNode.uncoveredEdges[*i_comboVerIter].end()) {
									vector<int> uncoverConV;
									uncoverConV.push_back(jointGraphNode.mapping[*j_comboVerIter]);
									jointNodeOrderNode.uncoveredEdges[*i_comboVerIter].insert(std::pair<int, vector<int>>(*i_comboVerIter, uncoverConV));
								}
								else {
									uncoverIter->second.push_back(jointGraphNode.mapping[*j_comboVerIter]);
								}
							}
						}
					}
				}

			}
		}
	}
}




/* u is the query vertex id, v is the data graph vertex id */
bool TurboIsoMQO::degreeFilter(int u, int v) {

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


bool  TurboIsoMQO::isJoinable(int u, int v) {

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


void TurboIsoMQO::updateState(int u, int v) {
	embedding[u] = v;
	inverseEmbedding.insert(std::pair<int, int>(v, u));
}

void TurboIsoMQO::restoreState(int u) {
	inverseEmbedding.erase(embedding[u]);
	embedding[u] = -1;
}