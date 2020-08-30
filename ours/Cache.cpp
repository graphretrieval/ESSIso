#include <iostream>
#include <map>
#include <queue>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <chrono>

#include "Cache.hpp"
#include "Graph.hpp"
#include "KDTree.hpp"

Graph * Cache::getGraphById(int i) {
    std::unordered_map<int, Graph>::iterator it = cachedGraphs.find(i);
    if (it == cachedGraphs.end()) return NULL;
    else return &(it->second);
}

std::vector<std::map<int,int>> * Cache::getEmbeddingsById(int i) {
    std::unordered_map<int, std::vector<std::map<int, int>>>::iterator it = cachedEmbeddings.find(i);
    if (it == cachedEmbeddings.end()) {
        std::cout << "EMBEDDING NULL" << std::endl;
        return NULL;
    } else return &(it->second);
}

void Cache::scanCache(Graph queryGraph, int *bestIndex, float* minDistance, std::unordered_map<int, float>* distanceMap) {
    nearestSearch(root, queryGraph.embeddingVector, bestIndex, minDistance, distanceMap);
}

void Cache::kScanCache(Graph queryGraph, std::priority_queue<std::pair<float, int>>* distanceQueue, std::unordered_map<int, float>* distanceMap, int k) {
    kNearestSearch(root, queryGraph.embeddingVector, distanceQueue, distanceMap, k);
}

void Cache::insertNode(int index, std::vector<float> point) {
    root = insertKDTree(root, index, point);
}

void Cache::deleteNode(int index, std::vector<float> point) {
    root = deleteKDTree(root, index, point);
}

void Cache::insert(Graph g, std::vector<std::map<int,int>> embeddings, float t, int index, std::unordered_map<int, float> distanceMap) {
    std::chrono::steady_clock::time_point cStart = std::chrono::steady_clock::now();
    if (cachedGraphs.size() >= maxCacheSize) 
        evictCache();
    cachedGraphs.insert({index, g});
    cachedEmbeddings.insert({index, embeddings});
    // frequentUse.insert({index,0});
    insertNode(index, g.embeddingVector);
    if (isLFU) {
        utilities.insert({index, 1});
    } else if (isLRU) {
        utilities.insert({index, index});
    } else {
        float c = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - cStart).count()/1000.0;
        tTime.insert({index, t+c});
        utilities.insert({index, t+c});        
    }

    if (cachedGraphs.size() >= maxCacheSize) {
        std::unordered_map<int, float>::iterator it = std::min_element(utilities.begin(), utilities.end(), 
        [](auto a, auto b){
            return (a.second < b.second); 
            });
        nextIndexToEvicted = it->first;
        minUtility = it->second;
    } else {
        nextIndexToEvicted = -1;
        minUtility = 0;
    }
}

void Cache::evictCache() {
    for (std::unordered_map<int, float>::iterator it = utilities.begin(); it != utilities.end(); it++) {
        it->second -= minUtility;
    }
    deleteNode(nextIndexToEvicted, cachedGraphs.find(nextIndexToEvicted)->second.embeddingVector);
    cachedGraphs.erase(nextIndexToEvicted);
    cachedEmbeddings.erase(nextIndexToEvicted);
    utilities.erase(nextIndexToEvicted);

}

void Cache::updateCacheHit(int index, float s, int matchIndex) {
    std::unordered_map<int,float>::iterator it = utilities.find(index);
    if (it!=utilities.end()) {
        if (isLFU) {
            it->second ++;
        } else if (isLRU) {
            it->second = matchIndex;
        } else {
            std::unordered_map<int,float>::iterator it2 = tTime.find(index);
            it->second += it2->second ;
        }        
    }

    if (cachedGraphs.size() >= maxCacheSize) {
        std::unordered_map<int, float>::iterator it = std::min_element(utilities.begin(), utilities.end(), 
            [](auto a, auto b){
                return (a.second < b.second); 
                });
        nextIndexToEvicted = it->first;
        minUtility = it->second;
    } else {
        nextIndexToEvicted = -1;
        minUtility = 0;
    }
}

std::unordered_map<int, Graph> Cache::getCacheGraphs() {
    return cachedGraphs;
}

std::unordered_map<int, std::vector<std::map<int, int>>> Cache::getCachedEmbeddings() {
    return cachedEmbeddings;
}

int Cache::size() {
    return cachedGraphs.size();
}

Cache::Cache(/* args */) {
}

Cache::Cache(int size) {
    maxCacheSize = size;
}

Cache::Cache(int size, bool lru, bool lfu) {
    maxCacheSize = size;
    isLRU = lru;
    isLFU = lfu;
}

Cache::~Cache() {
}
