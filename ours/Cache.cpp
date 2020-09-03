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
    if (brutal && distance) {
        std::vector<std::pair<int,float>> distanceQueue;
        for (const auto& i: distanceMap) {
            distanceQueue.push_back({i.first, i.second});
            cachedDistanceQueues[i.first].push_back({index, i.second});
            std::push_heap(cachedDistanceQueues[i.first].begin(), cachedDistanceQueues[i.first].end(), [](const std::pair<int, float>& a, const std::pair<int, float>& b) -> bool
            { 
                return a.second > b.second; 
            });
        }
        std::make_heap(distanceQueue.begin(), distanceQueue.end(), [](const std::pair<int, float>& a, const std::pair<int, float>& b) -> bool
            { 
                return a.second > b.second; 
            });    
        cachedDistanceQueues.insert({index, distanceQueue}); 
    }

    if (cachedGraphs.size() >= maxCacheSize) 
        evictCache();
    cachedGraphs.insert({index, g});
    cachedEmbeddings.insert({index, embeddings});
    cachedEmbeddingVectors.insert({index, g.embeddingVector});
    
    // frequentUse.insert({index,0});
    if (!brutal) {
        insertNode(index, g.embeddingVector);
    }
    if (isLFU) {
        utilities.insert({index, 1});
    } else if (isLRU) {
        utilities.insert({index, index});
    } else {
        float c = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - cStart).count()/1000.0;
        tTime.insert({index, t+c});
        if (!distance) {
            // std::cout << "Recache method" << std::endl;
            utilities.insert({index, t+c});
        } else {
            // std::cout << "Ours method" << std::endl;
            if (!brutal) {
                for (auto i: cachedGraphs) {
                    std::unordered_map<int, float> distanceMap;
                    std::priority_queue<std::pair<float, int>> distanceQueue;
                    kScanCache(i.second, &distanceQueue, &distanceMap, _k+1);
                    float distanceUtility = 0;
                    while(distanceQueue.size()>0) {
                        distanceUtility +=  distanceQueue.top().first;
                        distanceQueue.pop();
                    }      
                    distanceUtilities[i.first] = distanceUtility;
                    utilities[i.first] = tTime[i.first] * distanceUtility;
                }                 
            }
            else {
                for (const auto& i :cachedDistanceQueues) {
                    float distanceUtility = 0;
                    std::vector<std::pair<int, float>> temp = i.second;
                    for (int i =0; i < _k; i++) {
                        if (temp.size() ==0) {
                            break;
                        }
                        distanceUtility += temp.front().second;
                        std::pop_heap(temp.begin(), temp.end(), [](const std::pair<int, float>& a, const std::pair<int, float>& b) -> bool
                            { 
                                return a.second > b.second; 
                            });
                        temp.pop_back(); 
                    }
                    distanceUtilities[i.first] = distanceUtility;
                    utilities[i.first] = tTime[i.first] * distanceUtility;
                }
            }            
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

void Cache::evictCache() {
    for (std::unordered_map<int, float>::iterator it = utilities.begin(); it != utilities.end(); it++) {
        it->second -= minUtility;
    }
    std::cout << "Evict " << nextIndexToEvicted << std::endl;
    if (!brutal) {
        deleteNode(nextIndexToEvicted, cachedGraphs.find(nextIndexToEvicted)->second.embeddingVector);
    }
    cachedGraphs.erase(nextIndexToEvicted);
    cachedEmbeddings.erase(nextIndexToEvicted);
    cachedEmbeddingVectors.erase(nextIndexToEvicted);
    utilities.erase(nextIndexToEvicted);
    distanceUtilities.erase(nextIndexToEvicted);
    cachedDistanceQueues.erase(nextIndexToEvicted);
    for (auto& cachedDistanceQueue : cachedDistanceQueues) {
        for (int index = 0;index < cachedDistanceQueue.second.size();index++) {
            if (cachedDistanceQueue.second[index].first == nextIndexToEvicted) {
                cachedDistanceQueue.second.erase(cachedDistanceQueue.second.begin()+index);
                break;
            }
        }
        std::make_heap(cachedDistanceQueue.second.begin(), cachedDistanceQueue.second.end(), [](const std::pair<int, float>& a, const std::pair<int, float>& b) -> bool
        { 
            return a.second > b.second; 
        });
    }
}

void Cache::printCache() {
    for (auto i: cachedGraphs) {
        std::cout << "(" << i.first << "," << i.second.familyIndex << "," << utilities[i.first] << "," << tTime[i.first] <<"," << distanceUtilities[i.first] <<") ";
    }
    std::cout << std::endl;
    std::cout << "Min utility " << minUtility << std::endl;
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
            if (distance) {
                // std::cout << "Ours method" << std::endl;
                if (it2->second-s >0 )
                    it->second += (it2->second-s)*distanceUtilities[index] ;
            } else {
                // std::cout << "Recache method" << std::endl;
                if (it2->second-s)
                    it->second += it2->second-s;
            }                
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

Cache::Cache(int size, bool lru, bool lfu, bool distance_, int k) {
    maxCacheSize = size;
    isLRU = lru;
    isLFU = lfu;
    distance = distance_;
    _k = k;
}

Cache::Cache(int size, bool lru, bool lfu, bool distance_, bool brutal_, int k) {
    maxCacheSize = size;
    isLRU = lru;
    isLFU = lfu;
    distance = distance_;
    brutal = brutal_;
    _k = k;
}

Cache::~Cache() {
}
