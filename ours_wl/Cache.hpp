#ifndef CACHE_H

#define CACHE_H
#include <vector>
#include <map>
#include <unordered_map>
#include <queue>

#include "Graph.hpp"
// #include "KDTree.hpp"
class Cache {

public:
    int maxCacheSize = 1000;
    bool isLRU = false;
    bool isLFU = false;
    bool distance = false;
    bool brutal = false;
    int nextIndexToEvicted = -1;
    float minUtility = 0;
    int _k;
    std::unordered_map<int, float> tempUtilities;
    std::unordered_map<int, float> utilities;
    std::unordered_map<int, float> distanceUtilities;
    std::unordered_map<int, Graph> cachedGraphs;
    std::unordered_map<int, std::vector<float>> cachedEmbeddingVectors;
    std::unordered_map<int, std::vector<std::pair<int, float>>> cachedDistanceQueues;
private:
    struct Node *root = NULL; 
    // std::unordered_map<int, Graph> cachedGraphs;
    std::unordered_map<int, std::vector<std::map<int, int>>> cachedEmbeddings;
    void evictCache();
    std::unordered_map<int,int> frequentUse;
    std::unordered_map<int, float> tTime; 
    std::unordered_map<int, float> sTime; 

public:
    void kScanCache(Graph queryGraph, std::priority_queue<std::pair<float, int>>* distanceQueue, std::unordered_map<int, float>* distanceMap, int k);
    void scanCache(Graph queryGraph, int *bestIndex, float* minDistance, std::unordered_map<int, float>* distanceMap);
    void insertNode(int index, std::vector<float> point);
    void deleteNode(int index, std::vector<float> point);
    void printCache();
public:
    void updateCacheHit(int index, float s, int matchIndex);
    std::unordered_map<int, Graph> getCacheGraphs();
    std::unordered_map<int, std::vector<std::map<int, int>>> getCachedEmbeddings();
    Graph * getGraphById(int i);
    std::vector<std::map<int,int>>  * getEmbeddingsById(int i);
    void insert(Graph g, std::vector<std::map<int,int>> embeddings, float t,int index, std::unordered_map<int, float> distanceMap);
    int size();

public:
    Cache(/* args */);
    Cache(int size);
    Cache(int size, bool isLRU, bool isLFU, bool distance, int k);
    Cache(int size, bool isLRU, bool isLFU, bool distance, bool brutal, int k);
    ~Cache();
};

#endif