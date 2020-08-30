#include"ComboLinkedLists.h"
#include<vector>


using namespace std;

// Overload 
bool operator<(const CacheEmbeddingNode& first, const CacheEmbeddingNode& second) {
	for (int i = 0; i < 5; i++) {
		if (first.comboMappingVertices[i] < second.comboMappingVertices[i]) {
			return true;
		}
		else if (first.comboMappingVertices[i] > second.comboMappingVertices[i]) {
			return false;
		}
	}
	return false;
}

bool operator==(const CacheEmbeddingNode& first, const CacheEmbeddingNode& second) {
	bool result = true;
	for (int i = 0; i < 5; i++) {
		result == result && (first.comboMappingVertices[i] == second.comboMappingVertices[i]);
	}
	return result;
}

void ComboLinkedLists::addEmbeddingToCache(std::vector<std::vector<CacheEmbeddingNode *>> * cacheResults, AdjacenceListsGRAPH * queryGraph, int * embedding) {

	std::vector<ComboNODE> * comboGraph = queryGraph->getComboGraph();
	// std::cout << "Combo Graph of graph " << queryGraph->graphId << std::endl;
	// for (auto comboNode : *comboGraph) {
	// 	for (auto vertex: comboNode.queryVertices)
	// 		std::cout << vertex << " ";
	// 	std::cout << std::endl;
	// }
	// std::cout << "Embedding" << std::endl;
	// for (int i = 0; i < queryGraph->getNumberOfVertexes(); i++) 
	// 	std::cout << embedding[i] << " ";
	// std::cout << std::endl;

	short int ** comboGraphEdgeMatrix = queryGraph->getComboGraphEdgeMatrix();
	std::vector<int> * DFSComboVertexOrder = queryGraph->getDFSComboVertexOrder();
	std::vector<CacheEmbeddingNode *> queryVertexPositions;

	for (size_t i = 0; i<comboGraph->size(); i++) {
		// for (auto vertex: (*comboGraph)[i].queryVertices)
		// 	std::cout << vertex << " ";
		// std::cout << std::endl;

		CacheEmbeddingNode embeddingNode;
		for (size_t j = 0; j < (*comboGraph)[i].queryVertices.size(); j++) {
			embeddingNode.comboMappingVertices[j] = embedding[(*comboGraph)[i].queryVertices[j]];
		}

		/*
		* insert the node in an increasing order.
		* For each embedding docmo, we first find its inserting position
		*/
		CacheEmbeddingNode * insertPosition = (*cacheResults)[queryGraph->graphId][i];
		CacheEmbeddingNode * insertPositionParent = NULL;
		// std::cout << "Query Graph Id: " << queryGraph->graphId << std::endl;
		// std::cout << ((*insertPosition) < embeddingNode) << std::endl;

		while (insertPosition != NULL && (*insertPosition) < embeddingNode) {
			// std::cout << ((*insertPosition) < embeddingNode) << std::endl;
			insertPositionParent = insertPosition;
			insertPosition = insertPosition->adj;
		}

		if (insertPosition != NULL) {
			if ((*insertPosition) == embeddingNode) {
				queryVertexPositions.push_back(insertPosition);

			}
			else if (insertPosition == (*cacheResults)[queryGraph->graphId][i]) {
				(*cacheResults)[queryGraph->graphId][i] = new CacheEmbeddingNode(embeddingNode);
				(*cacheResults)[queryGraph->graphId][i]->adj = insertPosition;
				queryVertexPositions.push_back((*cacheResults)[queryGraph->graphId][i]);
			}
			else {
				insertPositionParent->adj = new CacheEmbeddingNode(embeddingNode);
				insertPositionParent->adj->adj = insertPosition;
				queryVertexPositions.push_back(insertPositionParent->adj);
			}

		}
		else {
			if (insertPositionParent != NULL) {
				insertPositionParent->adj = new CacheEmbeddingNode(embeddingNode);
				insertPositionParent->adj->adj = insertPosition;
				queryVertexPositions.push_back(insertPositionParent->adj);
			}
			else {
				(*cacheResults)[queryGraph->graphId][i] = new CacheEmbeddingNode(embeddingNode);
				(*cacheResults)[queryGraph->graphId][i]->adj = insertPosition;
				queryVertexPositions.push_back((*cacheResults)[queryGraph->graphId][i]);
			}
		}
		// std::cout << "queryVertexPositions" << std::endl;
		// for (int i = 0; i<5;i++)
		// 	std::cout << i << ":" << queryVertexPositions[queryVertexPositions.size()-1]->comboMappingVertices[i] << ";";

		// std::cout << "====================================" << std::endl;
	}

	// handle the dfs edges
	for (std::vector<int>::iterator orderComboVertexIterator = DFSComboVertexOrder->begin(); orderComboVertexIterator != DFSComboVertexOrder->end(); orderComboVertexIterator++) {
		for (std::vector<int>::iterator childrenIterator = (*comboGraph)[*orderComboVertexIterator].children.begin(); childrenIterator != (*comboGraph)[*orderComboVertexIterator].children.end(); childrenIterator++) {

			std::map<int, std::set<CacheEmbeddingNode *>>::iterator neighbourLinkIterator = queryVertexPositions[*orderComboVertexIterator]->neighboursList.find(*childrenIterator);

			if (neighbourLinkIterator != queryVertexPositions[*orderComboVertexIterator]->neighboursList.end()) {
				neighbourLinkIterator->second.insert(queryVertexPositions[*childrenIterator]);
			}
			else {
				set<CacheEmbeddingNode *> newList;
				newList.insert(queryVertexPositions[*childrenIterator]);
				queryVertexPositions[*orderComboVertexIterator]->neighboursList.insert(std::pair<int, set<CacheEmbeddingNode *>>(*childrenIterator, newList));
			}
		}
	}

	// handle the non-spanning(not in the dfs route) tree edges. the map is from (smaller id) point to the (larger id)
	for (size_t i = 0; i<comboGraph->size(); i++) {
		for (size_t j = i + 1; j<comboGraph->size(); j++) {
			if (comboGraphEdgeMatrix[i][j] && (*comboGraph)[i].parent != j && (*comboGraph)[j].parent != i) {

				std::map<int, std::set<CacheEmbeddingNode *>>::iterator neighbourLinkIterator = queryVertexPositions[i]->neighboursList.find(j);
				if (neighbourLinkIterator != queryVertexPositions[i]->neighboursList.end()) {
					neighbourLinkIterator->second.insert(queryVertexPositions[j]);
				}
				else {
					set<CacheEmbeddingNode *> newList;
					newList.insert(queryVertexPositions[j]);
					queryVertexPositions[i]->neighboursList.insert(std::pair<int, set<CacheEmbeddingNode *>>(j, newList));
				}
			}
		}
	}
}