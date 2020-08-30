#include <map>

#include "Graph.hpp"

struct biNodeSet {
    std::set<int> setG;
    std::set<int> setH;
    biNodeSet();
    biNodeSet(std::set<int> setG, std::set<int> setH);
};

class MCS {
public:
    Graph g;
    Graph h;

    std::map<int, int> mapping;
    std::map<int, int> incumbent;

public:
    std::map<int,int> solve( Graph & graph1,  Graph & graph2);
    std::map<int,int> solveWrapper(Graph & g_, Graph & h_);
    void showEmbedding();

private:
    std::map<std::string, biNodeSet> createOriginFuture();
    void search(std::map<std::string, biNodeSet> future);
    int calBound(std::map<std::string, biNodeSet> future); 
    std::pair<std::string, biNodeSet> selectLabelClass(std::map<std::string, biNodeSet> future);
    int selectVertex(std::set<int> v, const Graph & graph);
};