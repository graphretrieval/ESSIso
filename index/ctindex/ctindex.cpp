#include <string> 
#include <iostream>
#include <argp.h>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <functional>
#include <cassert>  

#include "Graph.hpp"

typedef std::vector<std::vector<int>> paths_t;

struct matrix_t {
    int n;
    std::vector<bool> data;

    matrix_t(int n) : n(n), data(n * n, false) {}

    bool get(int i, int j) const { return data[i * n + j]; }

    void set_true(int i, int j) { data[i * n + j] = true; }
    void set_false(int i, int j) { data[i * n + j] = false; }
};

struct tree_t {
    std::stack<int> vertexStack;
    std::vector<bool> include;
    std::stack<std::pair<int,int>> edgeStack;
    std::vector<std::unordered_set<int>> adjLists;
    matrix_t blockedEdges;
    int maxSize;

    tree_t(int n, int maxSize) : maxSize(maxSize), include(n, false), adjLists(n, std::unordered_set<int>()), blockedEdges(n) {}

    void addNode(int i) {
        vertexStack.push(i);
        include[i] = true;
    }

    void removeLastActiveNode() {
        int lastActiveNode = vertexStack.top();
        include[lastActiveNode] = false;
        for (const auto& neigh : adjLists[lastActiveNode]) {
            adjLists[neigh].erase(lastActiveNode);
        }
        adjLists[lastActiveNode].clear();
        vertexStack.pop();
    }

    void removeLastActiveEdge() {
        std::pair<int,int> lastActiveEdge = edgeStack.top();
        adjLists[lastActiveEdge.first].erase(lastActiveEdge.second);
        adjLists[lastActiveEdge.second].erase(lastActiveEdge.first);
        removeLastActiveNode();
    }

    std::vector<std::pair<int,int>> getExtensionEdges(const Graph& graph) {
        int n = include.size();
        std::vector<std::pair<int,int>> extensionEdges;
        if (vertexStack.size() >= maxSize) 
            return extensionEdges;

        for (int i =0 ;i < n; i++) {
            if (include[i]) {
                for (const auto& neigh : graph.adjList[i]) {
                    if (!include[neigh] && !blockedEdges.get(i,neigh) )
                        extensionEdges.push_back({i, neigh});
                }
            }
        }
        return extensionEdges;
    }

    void allowEdge(std::pair<int,int> edge) {
        blockedEdges.set_false(edge.first, edge.second);
        blockedEdges.set_false(edge.second, edge.first);
    }
    void blockEdge(std::pair<int,int> edge) {
        blockedEdges.set_true(edge.first, edge.second);
        blockedEdges.set_true(edge.second, edge.first);
    }
};

void print_trees(std::vector<std::string> results) {
    for (const auto& result : results) 
        std::cout << result << std::endl; 
}

void print_edges(std::vector<std::pair<int, int>> edges) {
    for (const auto& edge: edges) 
        std::cout << "( " << edge.first << " " << edge.second << ") ";
    std::cout << std::endl;
}
std::string canonize(const Graph& graph,const tree_t &tree) {
    int n = graph.adjList.size();
    tree_t tmp_tree = tree;
    std::unordered_set<int> vertexes;
    std::unordered_map<int, std::string> representation;

    for (int i =0; i < n; i++) 
        if (tmp_tree.include[i]) {
            vertexes.insert(i);
            representation.insert({i, std::to_string(graph.label[i])+"$"});
        }
    
    while(vertexes.size()>2) {
        std::vector<int> removeVertexes;
        for (int i =0; i < tmp_tree.adjLists.size(); i++) {
            if (tmp_tree.adjLists[i].size()==1) { // isLeaf
                removeVertexes.push_back(i);
            }
        } 
        std::unordered_map<int, std::vector<int>> childList;
        for (const auto & vertex :  removeVertexes) {
            int parent_vertex = *(tmp_tree.adjLists[vertex].begin());
            if (childList.find(parent_vertex) != childList.end()) {
                childList[parent_vertex].push_back(vertex);
            } else childList.insert({parent_vertex, {vertex}});

            tmp_tree.adjLists[parent_vertex].erase(vertex);
            tmp_tree.adjLists[vertex].erase(parent_vertex);
            vertexes.erase(vertex);
        }
        for (const auto & child : childList) {
            std::vector<std::string> childRepr;
            for (auto child_vertex: child.second)
                childRepr.push_back(representation[child_vertex]);
            std::sort(childRepr.begin(), childRepr.end());
            std::string childString = "";
            for (const auto& str : childRepr) {
                childString+= str;
            }
            std::string currentRepr = representation[child.first];
            representation[child.first] = currentRepr.substr(0, currentRepr.size()-1) + childString + "$";
        }
    }
    if (vertexes.size() ==1 ) 
        return representation[*(vertexes.begin())];
    else if (representation[*(vertexes.begin())] < representation[*(++vertexes.begin())])
        return representation[*(vertexes.begin())] + "$" + representation[*(++vertexes.begin())];
    else 
        return representation[*(++vertexes.begin())] + "$" + representation[*(vertexes.begin())];

}

void extendSubtreeByEdge(const Graph& graph, tree_t& tree, std::pair<int,int> edge, std::vector<std::string>& results) {
    tree.adjLists[edge.first].insert(edge.second);
    tree.adjLists[edge.second].insert(edge.first);
    tree.edgeStack.push(edge);
    if (tree.include[edge.first]) 
        tree.addNode(edge.second);
    else if (tree.include[edge.second])
        tree.addNode(edge.first);

    results.push_back(canonize(graph, tree));
    auto edges = tree.getExtensionEdges(graph);
    for (const auto& edge: edges) {
        extendSubtreeByEdge(graph, tree, edge, results);
    }
    for (const auto& edge: edges) {
        tree.allowEdge(edge);
    }
    tree.removeLastActiveEdge();
    tree.blockEdge(edge);
}

std::vector<std::string> find_all_trees(const Graph& graph) {
    int n = graph.adjList.size();
    std::vector<std::string> results;
    tree_t tree(n, 6);
    for (int i = 0 ; i<n;i++) {
        tree.addNode(i);
        auto result = canonize(graph, tree);
        results.push_back(result);
        for (const auto& edge: tree.getExtensionEdges(graph)) {

            int neigh_i;            
            if (edge.first ==i) 
                neigh_i=edge.second;
            else 
                neigh_i = edge.first;
            if (i > neigh_i) {
                extendSubtreeByEdge(graph, tree, {i, neigh_i}, results);
            }
        }
        tree.removeLastActiveNode();
    }
    std::sort(results.begin(), results.end());
    auto it = std::unique(results.begin(), results.end());
    results.erase(it, results.end());
    return results;
}

void linearize(std::vector<int>& path) {
    auto it = std::min_element(path.begin(), path.end());
    std::rotate(path.begin(), it, path.end());
}

void print(const std::vector<int>& path) {
    std::cout << "[ ";
    for (auto e: path) {
        std::cout << e << " ";
    }
    std::cout << "]" << std::endl;
}

void print_paths(const paths_t& paths) {
    std::cout << "=================================" << std::endl;
    for (auto& p: paths) {
        print(p);
    }
}

paths_t extend(Graph g, const paths_t& paths) {
    paths_t result;

    for (auto& p: paths) {
        auto end = p.back();
        for (auto v: g.adjList[end]) {
            std::vector<int> new_path = p;
            new_path.push_back(v);
            result.push_back(std::move(new_path));
        }
    }

    return result;
}

paths_t init_paths(Graph g) {
    paths_t paths;
    for (int u = 0; u < g.adjList.size(); u++) {
        for (auto v: g.adjList[u]) {
            paths.push_back({u, v});
        }
    }
    return paths;
}

paths_t find_all_cycles(Graph g) {
    paths_t tmp_result;
    paths_t result;
    paths_t paths = init_paths(g);

    for (int i = 3; i <= 9; i++) {
        if (paths.size()==0)
            break;
        paths = extend(g, paths);
        for (auto& p: paths) {
            if (p.front() == p.back()) {
                tmp_result.push_back(p);
            }
        }
        paths.erase(
            std::remove_if(paths.begin(), paths.end(),
                    [i](auto& p) { return 
                        (std::find(p.begin(),
                                p.end() - 1,
                                p.back()) != p.end() - 1)
                        || p.size() < i ;}),
            paths.end());

    }

    for (const auto& p : tmp_result) {
        if (p.size() <4) // if not path be like X A X -> just an edge
            continue;
        std::vector<int> new_p;
        for (auto it = p.begin(); it < p.end(); it++) {
            new_p.push_back(g.label[*it]);
        }
        result.push_back(new_p);
    }

    return result;
}

bool less_than(const std::vector<int>& a, const std::vector<int>& b) {
    return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
}

bool equals(const std::vector<int>& a, const std::vector<int>& b) {
    return std::equal(a.begin(), a.end(), b.begin(), b.end());
}

std::vector<int> tmp_path;

void linearize_paths(paths_t& paths) {
    for (auto& p: paths) {
        p.pop_back();

        linearize(p);
        tmp_path = p;
        std::reverse(tmp_path.begin(), tmp_path.end());
        linearize(tmp_path);

        if (!less_than(p, tmp_path)) {
            p = tmp_path;
        }
    }

    std::sort(paths.begin(), paths.end(),
            [](auto& a, auto& b) { return less_than(a, b); });

    auto it = std::unique(paths.begin(), paths.end(), 
            [](auto& a, auto& b) { return equals(a, b); });
    paths.erase(it, paths.end());
}

unsigned int hashFinger(std::string str, int maxVal) {
  std::hash<std::string> hasher;
  unsigned int hashed = hasher(str);
  return hashed%maxVal;
}

static void fail(std::string msg) {
    std::cerr << msg << std::endl;
    exit(1);
}

/*******************************************************************************
                             Command-line arguments
*******************************************************************************/

static char doc[] = "Find a maximum clique in a graph in DIMACS format\vHEURISTIC can be min_max or min_product";
static char args_doc[] = "HEURISTIC FILENAME1 FILENAME2 FILENAME3";
static struct argp_option options[] = {
    {"emb", 'e', 0, 0, "Use embedding"},
    {"cti", 'c', 0, 0, "Use ctindex"},
    {"wl", 'w', 0, 0, "Use wl"},
    { 0 }
};

static struct {
    bool directed;
    bool edge_labelled;
    bool vertex_labelled;
    char *datagraph;
    char *W_file;
    char *b_file;
    int arg_num;
    bool embedding;
    bool ctindex;
    bool wl;

} arguments;

void set_default_arguments() {
    arguments.directed = false;
    arguments.edge_labelled = false;
    arguments.vertex_labelled = true;
    arguments.datagraph = NULL;
    arguments.W_file = NULL;
    arguments.b_file = NULL;
    arguments.arg_num = 0;
    arguments.embedding = false;
    arguments.ctindex = false;
    arguments.wl = false;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
    switch (key) {
        case 'e':
            arguments.embedding = true;
            break;
        case 'c':
            arguments.ctindex = true;
            break;
        case 'w':
            arguments.wl = true;
            break;
        case ARGP_KEY_ARG:
            if (arguments.arg_num == 0) {
                arguments.datagraph = arg;
            } else if (arguments.arg_num == 1) {
                arguments.W_file = arg;
            } else if (arguments.arg_num == 2) {
                arguments.b_file = arg;
            } else {
                argp_usage(state);
            }
            arguments.arg_num++;
            break;
        case ARGP_KEY_END:
            if (arguments.arg_num == 0)
                argp_usage(state);
            break;
        default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

int main(int argc, char** argv) {
    unsigned long totalTime = 0;
    unsigned long totalScanTime = 0;
    int numberofFound = 0;
    int numberofFound2 = 0;
    set_default_arguments();
    argp_parse(&argp, argc, argv, 0, 0, 0);

    Graph dataGraph = readGraph(arguments.datagraph, arguments.directed,
    arguments.edge_labelled, arguments.vertex_labelled);
    // dataGraph.buildData();

    /* =================================================================
                        Embedding zone
    ====================================================================*/
    if (arguments.embedding) {
        FILE* f;
        
        if ((f=fopen(arguments.W_file, "r"))==NULL)
            fail("Cannot open file");

        char* line = NULL;
        size_t nchar = 0;
        std::vector<std::vector<float>> W;
        std::vector<float> b;
        while (getline(&line, &nchar, f) != -1) {
            if (nchar > 0) {
                std::stringstream s(line);
                std::string word; // to store individual words 
                std::vector<float> w;
                while (s >> word) {
                    w.push_back(stof(word));
                }
                W.push_back(w);
            }
        }

        if ((f=fopen(arguments.b_file, "r"))==NULL)
            fail("Cannot open file");

        while (getline(&line, &nchar, f) != -1) {
            if (nchar > 0) {
                std::stringstream s(line);
                std::string word; // to store individual words 
                std::vector<float> w;
                while (s >> word) {
                    b.push_back(stof(word));
                }
            }
        }
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        std::vector<std::vector<float>> features;
        int numberOfLabel = dataGraph.labelVertexList.size();
        for (int i=0;i<dataGraph.label.size();i++) {
            std::vector<float>nodeFeature, feature;
            for(int j =0 ; j < numberOfLabel; j++) {
                if (j==dataGraph.label[i])
                    nodeFeature.push_back(1);
                else nodeFeature.push_back(0);
            }
            for(int j =0 ; j < numberOfLabel; j++) {
                if (dataGraph.vertexlabelVertexList[i].find(j) != dataGraph.vertexlabelVertexList[i].end() )
                    nodeFeature.push_back(dataGraph.vertexlabelVertexList[i][j].size()/dataGraph.adjList[i].size());
                else nodeFeature.push_back(0);
            }
            
            for (int j = 0;j<W.size();j++) {
                float temp=0;
                for (int k=0;k<nodeFeature.size();k++) {
                    temp += W[j][k]*nodeFeature[k];
                }
                temp += b[j];
                feature.push_back(temp);
            }
            features.push_back(feature);
        }
        std::vector<float> embedding;
        for (int i =0;i<W.size();i++) {
            int temp=0;
            for (int j =0;j<features.size();j++)
                temp+= features[j][i];
            temp = temp/features.size();
            embedding.push_back(temp);
        }
        std::cout << arguments.datagraph << "\t" << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - begin).count() << std::endl; 
                    
    } 
    
    /* =================================================================
                        CTIndex zone
    ====================================================================*/
    if (arguments.ctindex) {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        std::unordered_set<unsigned int> hashIndexes;
        auto paths = find_all_cycles(dataGraph);
        linearize_paths(paths);
        for (const auto& path : paths) {
            std::string temp = "";
            for (const auto & i : path)
                temp += std::to_string(i);
            hashIndexes.insert(hashFinger(temp, 4096));
        }
        auto trees = find_all_trees(dataGraph);

        for (const auto& tree: trees) 
            hashIndexes.insert(hashFinger(tree, 4096));
        std::vector<float> embedding = std::vector<float>(4096, 0);
        for (const auto i : hashIndexes) {
            if (i >=4096) std::cout << i << " " << std::endl;
            else 
            embedding[(int)i] = 1.0;
        }
        // for (const auto i : hashIndexes)
        //     std::cout << i << " ";
        // std::cout << std::endl;
        std::cout << arguments.datagraph << "\t" << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - begin).count() << std::endl; 
        // print_paths(paths);       
    }

    /* =================================================================
                        WL zone
    ====================================================================*/
    if (arguments.wl) {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        std::map<std::pair<int,std::vector<int> >, int> wlFeatures; 
        for (int nodeIndex = 0; nodeIndex < dataGraph.label.size(); nodeIndex++) {
            std::map<int, std::vector<int>> vertexLabelVertex;
            int nodeLabel = dataGraph.label[nodeIndex];
            std::vector<int> neighLabels;
            for (int neighborIndex : dataGraph.adjList[nodeIndex]) {
                neighLabels.push_back(dataGraph.label[neighborIndex]);
            }
            std::sort(neighLabels.begin(), neighLabels.end());
            std::map<std::pair < int, std::vector<int> > , int>::iterator it = wlFeatures.find({nodeLabel, neighLabels});
            if (it == wlFeatures.end()) {
                wlFeatures.insert( {{nodeLabel, neighLabels}, 1} );
            } else {
                it->second += 1;
            }
        }

        std::cout << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - begin).count() << std::endl; 
        // print_paths(paths);       
    }

}
