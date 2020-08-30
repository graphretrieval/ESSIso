#include"TurboIsoBoostedMQO.h"
#include<queue>
#include<map>
#include<algorithm>
#include"CacheEmbeddingNode.h"
#include"ComboLinkedLists.h"
#include"CacheRetrieval.h"
#include"GlobalConstant.h"


TurboIsoBoostedMQO::TurboIsoBoostedMQO(AdjacenceListsGRAPH * pDataGraph, std::vector<std::vector<CacheEmbeddingNode *>> * pResultCaches, std::vector<int> * pNumberOfEmbeddings,
	std::vector<AdjacenceListsGRAPH> * pQueryGraphVector, std::vector<AdjacenceListsGRAPH> * pNewlyGeneratedGraphVector) {

	dataGraph = pDataGraph;

	numberOfEmbeddings = pNumberOfEmbeddings;
	resultCaches = pResultCaches;

	queryGraphVector = pQueryGraphVector;
	newlyGeneratedGraphVector = pNewlyGeneratedGraphVector;

}

TurboIsoBoostedMQO::~TurboIsoBoostedMQO() {

	jointNodeMatchingOrder = std::vector<JointNodeOrderNode>();
	inverseEmbedding = std::map<int, int>();
}

void TurboIsoBoostedMQO::setParameters(AdjacenceListsGRAPH * pQueryGraph, JointGraph * pJointGraph, bool pNeedSaveCache) {

	queryGraph = pQueryGraph;
	jointGraph = pJointGraph;
	needSaveCache = pNeedSaveCache;

	needReturn = false;
	recursiveCallsNo = 0;


	embedding = new int[queryGraph->getNumberOfVertexes()];
	for (int i = 0; i < queryGraph->getNumberOfVertexes(); i++) {
		embedding[i] = -1;
	}

	jointNodeMatchingOrder = std::vector<JointNodeOrderNode>();
	inverseEmbedding = std::map<int, int>();

}

void TurboIsoBoostedMQO::execute() {
	// we precompuete a matching order here.
	computeMatchingOrder();

	if (jointNodeMatchingOrder.size() != jointGraph->jointNodes.size()) {
		/*
		* some joint node doesn't have any candidates
		*/
		return;
	}


	JointGraphNode & jointNode = jointGraph->jointNodes[jointNodeMatchingOrder[0].jointNodeId];

	if (jointNode.parentId != -1) {
		AdjacenceListsGRAPH * parentGraph = NULL;
		// the starting node represents a parent graph
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
		cacheRetrieval->retrieveEmbeddingCandidate(queryGraph, parentGraph, &jointNode, &jointNodeMatchingOrder[0], embedding, &inverseEmbedding, &candidates);

		cout << "Start from Parent, candidate size: " << candidates.size() << endl << endl;

		for (std::vector<std::vector<CacheEmbeddingNode *>>::iterator candidateIter = candidates.begin(); candidateIter != candidates.end(); candidateIter++) {
			
			map<int, int> updatedQueryVertexMap;
			for (int comboNodeId = 0; comboNodeId < parentGraph->getComboGraph()->size(); comboNodeId++) {

				// each combo node corresponding to cache embedding node which contains multiple matched data vertices
				ComboNODE & comboNode = (*(parentGraph->getComboGraph()))[comboNodeId];
				for (int comboVertexIter = 0; comboVertexIter < comboNode.queryVertices.size(); comboVertexIter++) {

					int dataVertexId = (*candidateIter)[comboNodeId]->comboMappingVertices[comboVertexIter];
					int queryVertexId = jointNode.mapping[comboNode.queryVertices[comboVertexIter]];
					if (embedding[queryVertexId] == -1) {
						updateState(queryVertexId, dataVertexId);
						updatedQueryVertexMap.insert(std::pair<int, int>(dataVertexId, queryVertexId));
					}
				}
			}
			// next iteration
			resuriveSearch(1);
			// recover the last 
			for (map<int, int>::iterator updatedVIter = updatedQueryVertexMap.begin(); updatedVIter != updatedQueryVertexMap.end(); updatedVIter++) {
				restoreState(updatedVIter->first, updatedVIter->second);
			}
		}


		delete cacheRetrieval;
	}
	else {
		// the starting node represents a stand alone vertex
		AdjacenceListsGRAPH::Vertex * queryVertex = queryGraph->getVertexAddressByVertexId(jointNode.mapping[0]);
		std::map<int, std::vector<int>>::iterator candidates = dataGraph->getLabelVertexList()->find(queryVertex->label);
		for (vector<int>::iterator candidateIter = candidates->second.begin(); candidateIter != candidates->second.end(); candidateIter++) {

			// start with each candidate
			if (degreeFilter(queryVertex->id, *candidateIter)) {
				updateState(queryVertex->id, *candidateIter);
				resuriveSearch(1);
				restoreState(queryVertex->id, *candidateIter);
			}
		}
	}
}

void TurboIsoBoostedMQO::resuriveSearch(int matchingOrderIndex) {

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

			for (std::vector<std::vector<CacheEmbeddingNode *>>::iterator candidateIter = candidates.begin(); candidateIter != candidates.end(); candidateIter++) {

				map<int, int> updatedQueryVertexMap;
				for (int comboNodeId = 0; comboNodeId < parentGraph->getComboGraph()->size(); comboNodeId++) {

					// each combo node corresponding to cache embedding node which contains multiple matched data vertices
					ComboNODE & comboNode = (*(parentGraph->getComboGraph()))[comboNodeId];
					for (int comboVertexIter = 0; comboVertexIter < comboNode.queryVertices.size(); comboVertexIter++) {

						int dataVertexId = (*candidateIter)[comboNodeId]->comboMappingVertices[comboVertexIter];
						int queryVertexId = jointNode.mapping[comboNode.queryVertices[comboVertexIter]];
						if (embedding[queryVertexId] == -1) {
							updateState(queryVertexId, dataVertexId);
							updatedQueryVertexMap.insert(std::pair<int, int>(queryVertexId, dataVertexId));
						}
					}
				}
				// next iteration
				resuriveSearch(matchingOrderIndex + 1);
				// recover the last 
				for (map<int, int>::iterator updatedVIter = updatedQueryVertexMap.begin(); updatedVIter != updatedQueryVertexMap.end(); updatedVIter++) {
					restoreState(updatedVIter->first, updatedVIter->second);
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

			JointGraphNode & parentJointNode = jointGraph->jointNodes[jointNodeMatchingOrder[matchingOrderIndex - 1].jointNodeId];
			AdjacenceListsGRAPH::Vertex * parentMap = NULL;
			if (parentJointNode.parentId == -1) {
				// its parent is a stand alone vertex, we just get the mapped datavertex of its parent
				parentMap = dataGraph->getVertexAddressByVertexId(embedding[parentJointNode.mapping[0]]);
			}
			else {
				// its parent is a parent graph, we get a already mapped data vertex of a query vertex which is connected to this vertex
				for (size_t queryVertexIndex = 0; queryVertexIndex <= queryGraph->getNumberOfVertexes(); queryVertexIndex++) {
					if (embedding[queryVertexIndex] != -1 && queryGraph->edge(queryVertex->id, queryVertexIndex)) {
						parentMap = dataGraph->getVertexAddressByVertexId(embedding[queryVertexIndex]);
						break;
					}
				}
			}
			/*
			* we get is parent joint node, as its parent node has already been matched. we can use exploration to get the candidates
			*/
			std::map<int, std::vector<int>>::iterator exploreCandidates = parentMap->labelVertexList.find(queryVertex->label);
			if (exploreCandidates == parentMap->labelVertexList.end()) {
				// no candidate found
				return;
			}
			else {
				for (vector<int>::iterator candidateIter = exploreCandidates->second.begin(); candidateIter != exploreCandidates->second.end(); candidateIter++) {

					if (degreeFilter(queryVertex->id, *candidateIter) && isJoinable(queryVertex->id, *candidateIter)) {
						updateState(queryVertex->id, *candidateIter);
						resuriveSearch(matchingOrderIndex + 1);
						restoreState(queryVertex->id, *candidateIter);
					}
				}
			}
		}
	}
}

/*
* Given a PCM. we pre-compute a matching order here.
* to use the exploration similar to turboIso. we can define a BFS order
*/
void TurboIsoBoostedMQO::computeMatchingOrder() {

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
	for (size_t i = 0; i < jointGraph->jointNodes.size(); i++) {
		if (jointGraph->jointNodes[i].parentId != -1) {
			visited.push(i);
			flags[i] = true;
			startNodeOrderNode.jointNodeId = i;
			break;
		}
	}

	/*jointNodeOrderNode.jointNodeId = inverseMap[0].second;
	visited.push(inverseMap[0].second);
	flags[inverseMap[0].second] = true;*/
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



	}

	/*
	* add the common or connected already mapped query vertices
	*/
	for (int i = 0; i < jointNodeMatchingOrder.size(); i++) {

		JointNodeOrderNode & i_jointNodeOrderNode = jointNodeMatchingOrder[i];
		JointGraphNode & i_jointGraphNode = jointGraph->jointNodes[i_jointNodeOrderNode.jointNodeId];


		/*
		* initialize it
		*/
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
bool TurboIsoBoostedMQO::degreeFilter(int u, int v) {

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


bool  TurboIsoBoostedMQO::isJoinable(int u, int v) {

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


void TurboIsoBoostedMQO::updateState(int u, int v) {
	embedding[u] = v;
	inverseEmbedding.insert(std::pair<int, int>(v, u));
}

void TurboIsoBoostedMQO::restoreState(int u, int v) {
	embedding[u] = -1;
	inverseEmbedding.erase(v);
}