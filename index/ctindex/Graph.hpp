#ifndef COMMON_SUBGRAPH_H

#define COMMON_SUBGRAPH_H

#include <limits.h>
#include <stdbool.h>

#include <vector>
#include <set>
#include <map>

class Graph {

public:
    int n;
    int vectorSize;
    // std::vector<std::vector<unsigned int>> adjmat;
    std::vector<std::set<int>> adjList;
    std::vector<std::set<int>> nonadjList;
    std::vector<unsigned int> label;
    std::map<int, std::set<int>> labelVertexList;
    std::vector<std::map<int, std::vector<int>>> vertexlabelVertexList;
    std::map<int, int> degVertexList;
    std::vector<int> DFSTraversalOrder;
    std::vector<float> embeddingVector;

public:
    void buildData();

private:
    void buildLabelVertexList();
    void buildVertexLabelVertexList();
    void buildDegVertexList();
    void buildDFSTraversalOrder();

public:
    Graph();
    Graph(unsigned int n, int vectorSize);
    ~Graph();
};

Graph induced_subgraph(Graph& g, std::vector<int> vv);

Graph readGraph(char* filename, bool directed, bool edge_labelled, bool vertex_labelled);

#endif