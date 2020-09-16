#include "Graph.hpp"

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <stack>

constexpr int BITS_PER_UNSIGNED_INT (CHAR_BIT * sizeof(unsigned int));
int MAX_NODE = 50;

static void fail(std::string msg) {
    std::cerr << msg << std::endl;
    exit(1);
}

Graph::Graph(unsigned int n, int vectorSize) {
    this->n = n;
    this->vectorSize = vectorSize;
    label = std::vector<unsigned int>(n, 0u);
    // adjmat = {n, std::vector<unsigned int>(n, false)};
    adjList = {n, std::set<int>()};

    if (n < MAX_NODE) {
        std::set<int> tempSet;
        for (int i =0 ;i <n; i++)
            tempSet.insert(i);
        
        nonadjList = std::vector<std::set<int>>();
        for (int i =0 ;i <n; i++){
            nonadjList.push_back(tempSet);
            nonadjList[i].erase(i);
        }
    }

    labelVertexList = std::map<int, std::set<int>>();
    //TODO: Remove this hardcode
    vectorSize = 128;
    embeddingVector = std::vector<float>();  
}

Graph::Graph() {
}

Graph::~Graph() {

}

void Graph::buildData() {
    buildLabelVertexList();
    buildVertexLabelVertexList();
    buildDFSTraversalOrder();
    buildDegVertexList();
}

void Graph::buildLabelVertexList() {
    for (int nodeIndex = 0; nodeIndex < label.size(); nodeIndex++) {
        unsigned int nodeLabel = label[nodeIndex];

        if ( labelVertexList.find(nodeLabel) == labelVertexList.end() ) {
            labelVertexList.insert({nodeLabel, std::set<int>()});
        }
        labelVertexList[nodeLabel].insert(nodeIndex);

    }
}

void Graph::buildVertexLabelVertexList() {
    for (int nodeIndex = 0; nodeIndex < label.size(); nodeIndex++) {
        std::map<int, std::vector<int>> vertexLabelVertex;
        for (int neighborIndex : adjList[nodeIndex]) {
            std::map<int, std::vector<int>>::iterator vertexLabelVertexIterator = vertexLabelVertex.find(label[neighborIndex]);
            if (vertexLabelVertexIterator != vertexLabelVertex.end()) {
                vertexLabelVertexIterator->second.push_back(neighborIndex);
            } else {
                vertexLabelVertex.insert({label[neighborIndex], std::vector<int>({neighborIndex})});
            }
        }
        vertexlabelVertexList.push_back(vertexLabelVertex);
    }
}

void Graph::buildDegVertexList() {
    for (int nodeIndex = 0; nodeIndex < n; nodeIndex++) {
        int deg = 0;
        // for (std::vector<unsigned int>::iterator it = adjmat[nodeIndex].begin();  it != adjmat[nodeIndex].end(); it++) {
        //     deg += *it;
        // }

        degVertexList.insert({nodeIndex, adjList[nodeIndex].size()});
    }
}

void Graph::buildDFSTraversalOrder() {
    if (DFSTraversalOrder.size() > 0) {
        return;
    }

    std::vector<bool> flags(this->n, false);
    std::stack<int> S;
    int id;

    S.push(0);

    while(!S.empty()){

        id = S.top();
        S.pop();

        if(!flags[id]){
            flags[id] = true;
            DFSTraversalOrder.push_back(id);

            for (std::set<int>::iterator neighNodeIndex = adjList[id].begin(); neighNodeIndex != adjList[id].end(); neighNodeIndex++) {
                if (!flags[*neighNodeIndex])
                    S.push(*neighNodeIndex);
            } 
        }
    }
}

void add_edge(Graph& g, int v, int w, bool directed=false, unsigned int val=1) {
    if (v != w) {
        if (directed) {
            // g.adjmat[v][w] = val;
            g.adjList[v].insert(w);
            if (g.n < MAX_NODE)
                g.nonadjList[v].erase(w);
        } else {
            // g.adjmat[v][w] = val;
            // g.adjmat[w][v] = val;
            g.adjList[v].insert(w);
            g.adjList[w].insert(v);
            if (g.n < MAX_NODE) {
                g.nonadjList[v].erase(w);
                g.nonadjList[w].erase(v);
            }
        }
    } else {
        // To indicate that a vertex has a loop, we set the most
        // significant bit of its label to 1
        g.label[v] |= (1u << (BITS_PER_UNSIGNED_INT-1));
    }
}

Graph readDimacsGraph(char* filename, bool directed, bool vertex_labelled) {
    Graph g(0, 0);

    FILE* f;
    
    if ((f=fopen(filename, "r"))==NULL)
        fail("Cannot open file");

    char* line = NULL;
    size_t nchar = 0;

    int nvertices = 0;
    int medges = 0;
    int vectorSize = 0;
    int v, w;
    int edges_read = 0;
    int label;
    int familyIndex;
    while (getline(&line, &nchar, f) != -1) {
        if (nchar > 0) {
            switch (line[0]) {
            case 'p':
                if (sscanf(line, "p edge %d %d %d %d", &nvertices, &medges, &vectorSize, &familyIndex)!=4)
                    fail("Error reading a line beginning with p.\n");
                g = Graph(nvertices, vectorSize);
                g.familyIndex = familyIndex;
                break;
            case 'e':
                if (sscanf(line, "e %d %d", &v, &w)!=2)
                    fail("Error reading a line beginning with e.\n");
                add_edge(g, v, w, directed);
                edges_read++;
                break;
            case 'n':
                if (sscanf(line, "n %d %d", &v, &label)!=2)
                    fail("Error reading a line beginning with n.\n");
                if (vertex_labelled)
                    g.label[v] = label;
                break;
            default:
                std::stringstream s(line);
                std::string word; // to store individual words 
                while (s >> word) {
                    g.embeddingVector.push_back(stof(word));
                }
                if (g.embeddingVector.size() != g.vectorSize) 
                    fail("Embedding size mismatch.\n");
            }
        }
    }

    if (medges>0 && edges_read != medges) fail("Unexpected number of edges.");

    fclose(f);
    return g;
}

int read_word(FILE *fp) {
    unsigned char a[2];
    if (fread(a, 1, 2, fp) != 2)
        fail("Error reading file.\n");
    return (int)a[0] | (((int)a[1]) << 8);
}

struct Graph readGraph(char* filename, bool directed, bool edge_labelled, bool vertex_labelled) {
    Graph g(0,0);
    g = readDimacsGraph(filename, directed, vertex_labelled);
    return g;
}
