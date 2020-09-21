#include <string> 
#include <iostream>
#include <argp.h>
#include <algorithm>
#include <chrono>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <limits>
#include <queue>
#include <stack>
#include "TurboIso.hpp"
#include "Graph.hpp"
#include "SubGraphSearch.hpp"
#include "CommonSubGraph.hpp"
#include "Utils.hpp"
#include "Cache.hpp"

static void fail(string msg) {
    cerr << msg << endl;
    exit(1);
}

/*******************************************************************************
                             Command-line arguments
*******************************************************************************/

static char doc[] = "Find a maximum clique in a graph in DIMACS format\vHEURISTIC can be min_max or min_product";
static char args_doc[] = "HEURISTIC FILENAME1 FILENAME2 FILENAME3";
static struct argp_option options[] = {
    {"lfu", 'f', 0, 0, "Use lfu cache"},
    {"lru", 'r', 0, 0, "Use lru cache"},
    {"size", 's', "size", 0, "Specify a max cahce size"},
    {"cache", 'c', 0, 0, "Use cache"},
    {"use case 2", 'h', 0, 0, "Use case 2"},
    {"brutal-force search", 'b', 0, 0, "Use brutal-force search"},
    {"vf2", 'v', 0, 0, "Use vf2"},
    {"use distance utility", 'd', 0, 0, "Use distance utility"},
    {"timeout", 't', "timeout", 0, "Specify a timeout (seconds)"},
    {"nresults", 'e', "nresults", 0, "Specify max number of results"},
    {"ncalls", 'a', "ncalls", 0, "Specify max number of calls"},
    {"omega", 'w', "omega", 0, "Specify omega value"},
    {"k", 'k', "k", 0, "Specify k value"},

    { 0 }
};

static struct {
    bool directed;
    bool edge_labelled;
    bool vertex_labelled;
    char *datagraph;
    char *querydir;
    char *coldstartdir;
    float timeout;
    int arg_num;
    int test_case;
    int maxResult;
    int maxRCall;
    bool useCache;
    bool lru;
    bool lfu;
    bool vf2;
    int cache_size;
    bool case2;
    bool distance;
    int omega;
    int k;
    bool brutal;
} arguments;

void set_default_arguments() {
    arguments.directed = false;
    arguments.edge_labelled = false;
    arguments.vertex_labelled = true;
    arguments.datagraph = NULL;
    arguments.querydir = NULL;
    arguments.coldstartdir = NULL;
    arguments.timeout = 0.0;
    arguments.arg_num = 0;
    arguments.test_case = 2;
    arguments.maxResult = 100;
    arguments.maxRCall = 5000;
    arguments.useCache = false;
    arguments.lru = false;
    arguments.lfu = false;
    arguments.vf2 = false;
    arguments.case2 = false;
    arguments.distance = false;
    arguments.cache_size = 100;
    arguments.omega = 2;
    arguments.k = 5;
    arguments.brutal = false;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
    switch (key) {
        case 'f':
            arguments.lfu = true;
            break;
        case 'r':
            arguments.lru = true;
            break;
        case 'v':
            arguments.vf2 = true;
            break;
        case 's':
            arguments.cache_size = std::stoi(arg);
            break;
        case 'c':
            arguments.useCache = true;
            break;
        case 'h':
            arguments.case2 = true;
            break;
        case 'd':
            arguments.distance = true;
            break;
        case 'b':
            arguments.brutal = true;
            break;
        case 't':
            cout << arg << endl;
            arguments.timeout = std::stof(arg);
            break;
        case 'e':
            arguments.maxResult = std::stoi(arg);
            break;        
        case 'a':
            arguments.maxRCall = std::stoi(arg);
            break;        
        case 'w':
            arguments.omega = std::stoi(arg);
            break;
        case 'k':
            arguments.k = std::stoi(arg);
            break;
        case ARGP_KEY_ARG:
            if (arguments.arg_num == 0) {
                arguments.datagraph = arg;
            } else if (arguments.arg_num == 1) {
                arguments.querydir = arg;           
            } else if (arguments.arg_num == 2) {
                arguments.coldstartdir = arg;  
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

bool endsWith(const std::string &mainStr, const std::string &toMatch) {
    if(mainStr.size() >= toMatch.size() &&
        mainStr.compare(mainStr.size() - toMatch.size(), toMatch.size(), toMatch) == 0)
        return true;
    else
        return false;
}

int main(int argc, char** argv) {
    set_default_arguments();
    argp_parse(&argp, argc, argv, 0, 0, 0);
    Graph dataGraph = readGraph(arguments.datagraph, arguments.directed,
    arguments.edge_labelled, arguments.vertex_labelled);
    dataGraph.buildData();
    for (const std::pair<std::pair<int, std::vector<int>>, int> & wlFeature: dataGraph.wlFeatures) {
        std::cout << "< " << wlFeature.first.first << ", (";
        for (const int& neighLabel : wlFeature.first.second) {
            std::cout << neighLabel << " ";
        }
        std::cout << ") > :" << wlFeature.second << std::endl;
    }
}
