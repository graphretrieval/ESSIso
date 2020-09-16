#include <vector>
#include <map>
#include <set>
#include <cmath>
#include<iostream>
#include "Utils.hpp"

using namespace std;

set<map<int,int>> ProjectEmbedding::crossProjectEmbedding(vector<map<int,int>> resultsGraph_A2B, map<int,int> mapNode_B2C) {
    set<map<int,int>> candidateBigU =  set<map<int,int>>();
    for (auto result_A2B = resultsGraph_A2B.begin(); result_A2B != resultsGraph_A2B.end(); result_A2B++) {
        map<int,int> temp_C2A = map<int,int>();
        for (auto p_B2C = mapNode_B2C.begin(); p_B2C!=mapNode_B2C.end(); p_B2C++) {
            temp_C2A[p_B2C->first] = (*result_A2B)[p_B2C->second];
        }
        candidateBigU.insert(temp_C2A);
    }
    return candidateBigU;
}

set<map<int,int>> ProjectEmbedding::projectEmbedding(set<map<int,int>> resultsGraph_A2B, set<int> nodes_A) {
    set<map<int,int>> candidateBigU =  set<map<int,int>>();
    for (auto result_A2B = resultsGraph_A2B.begin(); result_A2B != resultsGraph_A2B.end(); result_A2B++) {
        map<int,int> temp_subA2B = map<int,int>();
        for (auto p_A2B = result_A2B->begin(); p_A2B!=result_A2B->end(); p_A2B++) {
            if (nodes_A.find(p_A2B->first) != nodes_A.end())
                temp_subA2B.insert(*p_A2B);
        }
        candidateBigU.insert(temp_subA2B);
    }
    return candidateBigU;
}    

float ProjectEmbedding::distance (vector<float> vec1, vector<float> vec2) {
    int size = vec1.size();
    if (size > vec2.size())
        size = vec2.size();

    // float dist = 0;
    // for (int i = 0; i < size; i++) {
    //     dist += pow(vec1[i] - vec2[i], 2);
    // }
    // dist = sqrt(dist);
    // return dist;
    float A = 0;
    float B = 0;
    float C = 0;
    for (int i = 0; i < size; i++) {
        A += vec1[i] * vec2[i];
        B += pow(vec1[i], 2);
        C += pow(vec2[i], 2);
    }
    float dist = 1-A/(sqrt(B)*sqrt(C));
    return dist;
}