#pragma once

#include"AdjacenceListsGraph.h"
#include"CacheEmbeddingNode.h"
#include<vector>





class ComboLinkedLists {

public:

	static void addEmbeddingToCache(std::vector<std::vector<CacheEmbeddingNode *>> * cacheResults, AdjacenceListsGRAPH * queryGraph, int * embedding);

};


