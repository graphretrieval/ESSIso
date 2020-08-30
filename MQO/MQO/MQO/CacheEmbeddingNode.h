#pragma once
class CacheEmbeddingNode {

public:
	/*
	* At most 5 values
	*/
	int * comboMappingVertices;

	CacheEmbeddingNode * adj;

	std::map<int, std::set<CacheEmbeddingNode *>> neighboursList;

	CacheEmbeddingNode() {
		comboMappingVertices = new int[5];
		for (int i = 0; i<5; i++) {
			comboMappingVertices[i] = -1;
		}
		adj = NULL;
	};

	CacheEmbeddingNode(const CacheEmbeddingNode & pCacheEmbeddingNode) {
		comboMappingVertices = new int[5];
		for (int i = 0; i<5; i++) {
			comboMappingVertices[i] = pCacheEmbeddingNode.comboMappingVertices[i];
		}
		adj = pCacheEmbeddingNode.adj;
		neighboursList = pCacheEmbeddingNode.neighboursList;
	};

	~CacheEmbeddingNode() {
		neighboursList = std::map<int, std::set<CacheEmbeddingNode *>>();
		delete[] comboMappingVertices;
	}
};
