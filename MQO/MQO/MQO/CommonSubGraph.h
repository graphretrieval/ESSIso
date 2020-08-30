#pragma once
#include <map>
#include"AdjacenceListsGraph.h"

struct biNodeSet {
    std::set<int> setG;
    std::set<int> setH;
    biNodeSet();
    biNodeSet(std::set<int> setG, std::set<int> setH);
};

class MCS {
public:
    AdjacenceListsGRAPH g;
    AdjacenceListsGRAPH h;

    std::map<int, int> mapping;
    std::map<int, int> incumbent;

public:
    std::map<int,int> solve( AdjacenceListsGRAPH & graph1,  AdjacenceListsGRAPH & graph2);
    void showEmbedding();

private:
    std::map<std::string, biNodeSet> createOriginFuture();
    void search(std::map<std::string, biNodeSet> future);
    int calBound(std::map<std::string, biNodeSet> future); 
    std::pair<std::string, biNodeSet> selectLabelClass(std::map<std::string, biNodeSet> future);
    int selectVertex(std::set<int> v, AdjacenceListsGRAPH graph);
};