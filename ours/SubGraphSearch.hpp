#include <map>
#include <vector>
#include <set>
#include <chrono>

#include "Graph.hpp"

class SubGraphSearch {

public: 
    std::vector<std::map<int,int>> allMappings;
    Graph dataGraph, queryGraph;
    int maxEmbedding;
    std::map<int,int> largestMapping;
    int maxRCall;
    float timeOut;
private:
    std::chrono::steady_clock::time_point startTime;
    int nCall;
    bool needReturn;
    std::map<int, std::set<int> > candidates;
    std::map<int, int> mapping;
    std::map<int, int> reverseMapping;

public:
    std::vector<std::map<int,int>> getAllSubGraphsWrapper(const Graph & pdataGraph, const Graph &pqueryGraph, int maxRCall_, int maxEmbedding_, int timeout);
    std::vector<std::map<int,int>> getAllSubGraphs(const Graph & dataGraph, const Graph &queryGraph, int maxRCall, int maxEmbedding);
    std::vector<std::map<int,int>> getAllSubGraphsCase2(const Graph & dataGraph, const Graph &queryGraph, std::set<std::map<int,int>> candidateBigU,int maxRCall,  int maxEmbedding);
    std::vector<std::map<int,int>> getAllSubGraphsCase3(const Graph & dataGraph, const Graph &queryGraph, std::set<std::map<int,int>> candidateBigU);
    void showEmbedding();

private:
    void filterCandidates();
    void isomorphismSearch();
    int nextQueryVertex();
    bool refineCandidates(int u, int v);
    bool degreeFilter(int u, int v);
    bool isJoinable(int u, int v);
    void updateState(int u, int v);
    void restoreState(int u, int v);
    
};

