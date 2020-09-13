#include"MQO.h"
#include"TimeUtility.h"
#include"MatrixUtility.h"
#include<queue>
#include"QueryExecutionOrder.h"
#include<algorithm>

using namespace std;

MQO::MQO() {
	numOfFound = 0;
}

MQO::MQO(std::vector<AdjacenceListsGRAPH> * pDataGraphVector, std::vector<AdjacenceListsGRAPH> * pQueryGraphVector, std::ofstream * pResultFile) {

	resultFile = pResultFile;

	if (pDataGraphVector->size() > 0) {
		dataGraph = &(*pDataGraphVector)[0];
	}

	queryGraphVector = pQueryGraphVector;

	/*
	* use a matrix to save the tls information between queries
	*/
	tlsGraphMatrix = new float*[queryGraphVector->size()];
	for (unsigned int i = 0; i < queryGraphVector->size(); i++) {
		tlsGraphMatrix[i] = new float[queryGraphVector->size()];
		for (unsigned int j = 0; j < queryGraphVector->size(); j++) {
			tlsGraphMatrix[i][j] = 0;
		}
	}

	/*
	* create pcm node for each query graph
	*/
	for (unsigned int i = 0; i < queryGraphVector->size(); i++) {
		patternContainmentMap.push_back(PCM_Node(i, false));
	}

	/*
	 * create instances for subgraph isomorphism
	 */
	turboIso = new TurboIso(dataGraph, &numberOfEmbeddings, &resultCaches);
	turboIsoMQO = new TurboIsoMQO(dataGraph, &resultCaches, &numberOfEmbeddings, queryGraphVector, &newlyGeneratedGraphVector);
	turboIsoBoosted = new TurboIsoBoosted(dataGraph, &numberOfEmbeddings, &resultCaches);
	turboIsoBoostedMQO = new TurboIsoBoostedMQO(dataGraph, &resultCaches, &numberOfEmbeddings, queryGraphVector, &newlyGeneratedGraphVector);
}

MQO::~MQO() {

	for (unsigned int i = 0; i < queryGraphVector->size(); ++i) {
		delete[] tlsGraphMatrix[i];
	}
	delete[] tlsGraphMatrix;

}


void MQO::buildPCM() {

	/*
	 * Build the tls graph matrix
	 */
	TLSGraph * tlsGraph = new TLSGraph(tlsGraphMatrix, queryGraphVector, resultFile);
	tlsGraph->buildTLSGraph(0);
	delete tlsGraph;

// #ifdef _DEBUG
	std::cout << endl << "TLS Simi Matrix " << endl;
	MatrixUtility::outputFloatMatrix(tlsGraphMatrix, (*queryGraphVector).size(), (*queryGraphVector).size(), &std::cout);
// #endif

	PCMBuilder * pcmBuilder = new PCMBuilder(tlsGraphMatrix, &patternContainmentMap, queryGraphVector, &newlyGeneratedGraphVector, &std::cout);

	pcmBuilder->execute();

	delete pcmBuilder;
}

void MQO::buildQueryExecutionOrder() {
	
	QueryExecutionOrder * queryExecutionOrder = new QueryExecutionOrder(&patternContainmentMap, &hybridExecutionOrder);
	queryExecutionOrder->computeExecutionOrder();
	delete queryExecutionOrder;

// #ifdef _DEBUG
	std::cout << endl << "Query Execution Order: ";
	for (std::vector<ExecutionOrderNode>::iterator orderIterator = hybridExecutionOrder.begin(); orderIterator != hybridExecutionOrder.end(); orderIterator++) {
		std::cout << orderIterator->pcmId << "-> Release < ";
		for (vector<int>::iterator parentIterator = orderIterator->releaseParentsPcmId.begin(); parentIterator != orderIterator->releaseParentsPcmId.end(); parentIterator++) {
			std::cout << *parentIterator << ",";
		}
		std::cout << ">, ";
	}
	// cout << endl << endl;
// #endif
}


/*
 * the build joint graph only heuritically selected some of its parents based on PCM. however, the joint node order hasn't not computed.
 * the reazon why is because the each subgraph isomorphism may has its own order approach.
 */
void MQO::buildJointGraph(ExecutionOrderNode * orderNode) {
	
	jointGraph.clear();
	/*
	 * get a ordered parents based on the size of cache of its parents
	 */
	vector<std::pair<int, int>> parentsEmbeddingSize;
	for (vector<int>::iterator parentIter = patternContainmentMap[orderNode->pcmId].parent.begin(); parentIter != patternContainmentMap[orderNode->pcmId].parent.end(); parentIter++) {
		parentsEmbeddingSize.push_back(std::pair<int, int> (numberOfEmbeddings[*parentIter], *parentIter));
	}
	std::sort(parentsEmbeddingSize.begin(), parentsEmbeddingSize.end());
	
	/*
	 * use a simple heuristic here. less number comes first.
	 */
	set<int> coveredVertices;
	for (vector<std::pair<int, int>>::iterator parentIter = parentsEmbeddingSize.begin(); parentIter != parentsEmbeddingSize.end(); parentIter++) {
		if (coveredVertices.size() == queryGraph->getNumberOfVertexes()) {
			break;
		}
		std::map<int, std::vector<std::vector<int>>>::iterator parentMappings = patternContainmentMap[parentIter->second].containmentRelationshipMappingLists.find(orderNode->pcmId);
		/*
		 * a parent may has multiple mappings to this query
		 */
		for (std::vector<std::vector<int>>::iterator parentMappingsIter = parentMappings->second.begin(); parentMappingsIter != parentMappings->second.end(); parentMappingsIter++) {
			if (coveredVertices.size() == queryGraph->getNumberOfVertexes()) {
				break;
			}
			bool foundNewVertices = false;
			for (std::vector<int>::iterator vertexIter = parentMappingsIter->begin(); vertexIter != parentMappingsIter->end(); vertexIter++) {
				if (coveredVertices.find(*vertexIter) == coveredVertices.end()) {
					foundNewVertices = true;
					break;
				}
			}
			if (foundNewVertices) {
				JointGraphNode jointGraphNode;
				jointGraphNode.parentId = parentIter->second;
				jointGraphNode.mapping = (*parentMappingsIter);
				jointGraph.jointNodes.push_back(jointGraphNode);

				coveredVertices.insert(parentMappingsIter->begin(), parentMappingsIter->end());
			}
		}
	}
	/*
	 * add the uncovered vertex as stand-alone nodes
	 */
	for (int querVertex = 0; querVertex < queryGraph->getNumberOfVertexes(); querVertex++) {
		if (coveredVertices.find(querVertex) == coveredVertices.end()) {
			JointGraphNode jointGraphNode;
			jointGraphNode.parentId = -1;
			jointGraphNode.mapping.push_back(querVertex);
			jointGraph.jointNodes.push_back(jointGraphNode);
		}
	}
	


	/*
	 * add joint graph edges
	 */
	for (int fromIndex = 0; fromIndex < jointGraph.jointNodes.size(); fromIndex++) {
		JointGraphNode & fromNode = jointGraph.jointNodes[fromIndex];
		for (int toIndex = fromIndex + 1; toIndex < jointGraph.jointNodes.size(); toIndex++) {
			JointGraphNode & toNode = jointGraph.jointNodes[toIndex];
			bool connected = false;
			for (vector<int>::iterator fromNodeMappingIter = fromNode.mapping.begin(); fromNodeMappingIter != fromNode.mapping.end(); fromNodeMappingIter++) {
				if (connected) {
					break;
				}
				for (vector<int>::iterator toNodeMappingIter = toNode.mapping.begin(); toNodeMappingIter != toNode.mapping.end(); toNodeMappingIter++) {
					if (queryGraph->edge(*fromNodeMappingIter, *toNodeMappingIter) || *fromNodeMappingIter == *toNodeMappingIter) {
						connected = true;
						break;
					}
				}
			}
			if (connected) {
				jointGraph.insertEdge(fromIndex, toIndex);
			}
		}
	}
}


void MQO::orederedQueryProcessing() {
	/*
	 * initialize cache embedding for all the PCM node
	 */
	resultCaches = vector<vector<CacheEmbeddingNode*>>(patternContainmentMap.size());
	numberOfEmbeddings = vector<int>(patternContainmentMap.size());

	for (unsigned int i = 0; i < patternContainmentMap.size(); i++) {

		numberOfEmbeddings[i] = 0;

		AdjacenceListsGRAPH * tempQueryGraph = NULL;
		if (i < queryGraphVector->size()) {
			tempQueryGraph = &(*queryGraphVector)[i];
		}
		else {
			tempQueryGraph = &newlyGeneratedGraphVector[i - queryGraphVector->size()];
		}

		resultCaches[i] = vector<CacheEmbeddingNode*>(tempQueryGraph->getComboGraph()->size());
		for (unsigned int j = 0; j < tempQueryGraph->getComboGraph()->size(); j++) {
			resultCaches[i][j] = NULL;
		}
	}
	
	/*
	 * for each pcm node according to the pre-computed execution order
	 */
	for (std::vector<ExecutionOrderNode>::iterator orderIterator = hybridExecutionOrder.begin(); orderIterator != hybridExecutionOrder.end(); orderIterator++) {
		// std::cout << "1" << std::endl;
		if(orderIterator->pcmId < queryGraphVector->size()){
			queryGraph = &(queryGraphVector->at(orderIterator->pcmId));
		}
		else {
			queryGraph = &(newlyGeneratedGraphVector.at(orderIterator->pcmId - queryGraphVector->size()));
		}

		bool needSaveCache = patternContainmentMap[orderIterator->pcmId].children.size() == 0 ? false : true;
		// std::cout << "needSaveCache: " << needSaveCache << std::endl;
		// needSaveCache = true;
		// std::cout << "2" << std::endl;	

		if (true) {
		// if (patternContainmentMap[orderIterator->pcmId].parent.size() == 0) {
			/*
			 * this query has no pcm parents, thus we can only use the original subgraph isomorphism algorithms
			 */
			switch (GlobalConstant::G_RUNNING_OPTION_INDEX) {
				case GlobalConstant::RUN_OP_TURBOISO: {
					// std::cout << "turbosio" << std::endl;
					turboIso->setParameters(queryGraph, needSaveCache);
					try {
						// std::cout << "Execute" << std::endl;
						turboIso->execute();
					}
					catch(std::runtime_error& e) {
						std::cout << e.what() << std::endl;
					}
					// turboIso->execute();
// #ifdef _DEBUG
					std::cout << "Turboiso finished one query " << orderIterator->pcmId << ": " << numberOfEmbeddings[orderIterator->pcmId] << endl;
// #endif
					break;
				}
				case GlobalConstant::RUN_OP_TURBO_ISO_BOOSTED: {
					turboIsoBoosted->setParameters(queryGraph, needSaveCache);
					turboIsoBoosted->execute();
					break;
				}
				default: { // handle when option is EmBoost
					// std::cout << "turbosio" << std::endl;
					turboIso->setParameters(queryGraph, needSaveCache);
					try {
						// std::cout << "Execute" << std::endl;
						turboIso->execute();
					}
					catch(std::runtime_error& e) {
						std::cout << e.what() << std::endl;
					}
					// turboIso->execute();
					std::cout << "Turboiso finished one query " << orderIterator->pcmId << ": " << numberOfEmbeddings[orderIterator->pcmId] << endl;
					break;
				}
			}
		}
		else {
			/*
			 * this query has pcm parents, we could use multi-query optimization
			 */
			buildJointGraph(&(*orderIterator));
			switch (GlobalConstant::G_RUNNING_OPTION_INDEX) {
				case GlobalConstant::RUN_OP_TURBOISO: {
					// std::cout << "turboIsoMQO" << std::endl;
					turboIsoMQO->setParameters(queryGraph, &jointGraph, needSaveCache);
					try {
						// std::cout << "Execute" << std::endl;
						turboIsoMQO->execute();
					}
					catch(std::runtime_error& e) {
						std::cout << e.what() << std::endl;
					}
					// turboIsoMQO->execute();
// #ifdef _DEBUG
					std::cout << "TurboMQO finished one query: " << orderIterator->pcmId << ": " << numberOfEmbeddings[orderIterator->pcmId] << endl;
// #endif
					break;
				}
				case GlobalConstant::RUN_OP_TURBO_ISO_BOOSTED: {
					turboIsoBoostedMQO->setParameters(queryGraph, &jointGraph, needSaveCache);
					turboIsoBoostedMQO->execute();
					break;
				}
				default: { // handle when option is EmBoost
					// std::cout << "turboIsoMQO" << std::endl;
					turboIsoMQO->setParameters(queryGraph, &jointGraph, needSaveCache);
					try {
						// std::cout << "Execute" << std::endl;
						turboIsoMQO->execute();
					}
					catch(std::runtime_error& e) {
						std::cout << e.what() << std::endl;
					}
					// turboIsoMQO->execute();
					std::cout << "TurboMQO finished one query: " << orderIterator->pcmId << ": " << numberOfEmbeddings[orderIterator->pcmId] << endl;
					break;
				}
			}
		}
		if (numberOfEmbeddings[orderIterator->pcmId]>0)
			numOfFound += 1;
		/*
		 * release the memory
		 */
		for (vector<int>::iterator releaseParentIter = orderIterator->releaseParentsPcmId.begin(); releaseParentIter != orderIterator->releaseParentsPcmId.end(); releaseParentIter++) {
			resultCaches[*releaseParentIter] = std::vector<CacheEmbeddingNode *>();
		}
	}
}

