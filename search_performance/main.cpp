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

static void fail(string msg) {
    cerr << msg << endl;
    exit(1);
}

/*******************************************************************************
                             Command-line arguments
*******************************************************************************/

static char doc[] = "Find a maximum clique in a graph in DIMACS format\vHEURISTIC can be min_max or min_product";
static char args_doc[] = "HEURISTIC FILENAME1 FILENAME2";
static struct argp_option options[] = {
    {"vf2", 'v', 0, 0, "Use vf2"},
    {"timeout", 't', "timeout", 0, "Specify a timeout (seconds)"},

    {"k", 'k', "k", 0, "Specify k value"},
    { 0 }
};

static struct {
    bool directed;
    bool edge_labelled;
    bool vertex_labelled;
    char *datadir;
    char *querydir;
    float timeout;
    int arg_num;
    bool vf2;
    int k;
    bool brutal;
} arguments;

void set_default_arguments() {
    arguments.directed = false;
    arguments.edge_labelled = false;
    arguments.vertex_labelled = true;
    arguments.datadir = NULL;
    arguments.querydir = NULL;
    arguments.timeout = 0.0;
    arguments.arg_num = 0;
    arguments.vf2 = false;
    arguments.k = 5;
    arguments.brutal = false;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
    switch (key) {
        case 'v':
            arguments.vf2 = true;
            break;
        case 't':
            arguments.timeout = std::stof(arg);
            break;
        case 'k':
            arguments.k = std::stoi(arg);
            break;
        case ARGP_KEY_ARG:
            if (arguments.arg_num == 0) {
                arguments.datadir = arg;
            } else if (arguments.arg_num == 1) {
                arguments.querydir = arg;           
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

    std::vector<Graph> dataGraphVector;
    std::vector<std::vector<float>> dataGraphEmbeddings;
    DIR* dirp = opendir(arguments.datadir);
    struct dirent * dp;
    std::string datadir(arguments.datadir);
    std::vector<std::string> filenames;
    while ((dp = readdir(dirp)) != NULL) {
        if (dp->d_name[0] == 'q' && endsWith(dp->d_name, "dimas")) {
            std::string filename(dp->d_name);
            filenames.push_back(datadir+filename);
        }
    }
    closedir(dirp);
    sort(filenames.begin(), filenames.end());
    for (std::string filename : filenames) {
        char c_filename[filename.size() + 1];
	    strcpy(c_filename, filename.c_str());
        Graph dataGraph = readGraph(c_filename, arguments.directed,
        arguments.edge_labelled, arguments.vertex_labelled);
        dataGraph.buildData();
        dataGraphVector.push_back(dataGraph);   
        dataGraphEmbeddings.push_back(dataGraph.embeddingVector);  
    }
    std::cout << "Got " << dataGraphVector.size() << " query graph(s)" << std::endl;

    std::vector<Graph> queryGraphVector;
    std::vector<std::vector<float>> queryGraphEmbeddings;

    dirp = opendir(arguments.querydir);
    std::string querydir(arguments.querydir);
    filenames.clear();
    while ((dp = readdir(dirp)) != NULL) {
        if (dp->d_name[0] == 'q' && endsWith(dp->d_name, "dimas")) {
            std::string filename(dp->d_name);
            filenames.push_back(querydir+filename);
        }
    }
    closedir(dirp);
    sort(filenames.begin(), filenames.end());
    for (std::string filename : filenames) {
        char c_filename[filename.size() + 1];
	    strcpy(c_filename, filename.c_str());
        Graph queryGraph = readGraph(c_filename, arguments.directed,
        arguments.edge_labelled, arguments.vertex_labelled);
        queryGraph.buildData();
        queryGraphVector.push_back(queryGraph);   
        queryGraphEmbeddings.push_back(queryGraph.embeddingVector);  
    }
    std::cout << "Got " << queryGraphVector.size() << " query graph(s)" << std::endl;

    SubGraphSearch subGraphSlover = SubGraphSearch();
    subGraphSlover.timeOut = arguments.timeout;
    TurboIso turboIsoSlover = TurboIso();
    turboIsoSlover.timeOut = arguments.timeout;
    MCS mcsSolver = MCS();

    float timeWithoutEmb = 0;
    float timeWithEmb = 0;
    int correctWithoutEmb = 0;
    int correctWithEmb = 0;
    float runTime = 0;
    std::chrono::steady_clock::time_point startTime;
    for (int i=0; i < queryGraphVector.size(); i++ ) {
        Graph queryGraph = queryGraphVector[i];
        std::vector<float> queryEmbedding = queryGraphEmbeddings[i];
        
        int maxMCS = 0;
        int bestId = -1;

        startTime = std::chrono::steady_clock::now();
        for ( Graph& dataGraph: dataGraphVector ) {
            std::map<int,int> tempMapNode;
            if (arguments.vf2) {
                if (queryGraph.n <= dataGraph.n) {
                    try {
                        subGraphSlover.getAllSubGraphs(dataGraph,queryGraph, 0, 1);
                    }
                    catch(std::runtime_error& e) {
                        std::cout << "Time out" << std::endl;
                    }
                } else {
                    try {
                        subGraphSlover.getAllSubGraphs(queryGraph,dataGraph, 0, 1);
                    }
                    catch(std::runtime_error& e) {
                        std::cout << "Time out" << std::endl;
                    }
                }
                if (maxMCS < subGraphSlover.largestMapping.size()) {
                    maxMCS = subGraphSlover.largestMapping.size();
                    bestId = dataGraph.familyIndex;
                    if (maxMCS == queryGraph.n) {
                        break;
                    }
                }                
            } else {
                if (queryGraph.n <= dataGraph.n) {
                    try {
                        turboIsoSlover.getAllSubGraphs(dataGraph,queryGraph, 0, 1);
                    }
                    catch(std::runtime_error& e) {
                        std::cout << "Time out" << std::endl;
                    }
                } else {
                    try {
                        turboIsoSlover.getAllSubGraphs(queryGraph,dataGraph, 0, 1);
                    }
                    catch(std::runtime_error& e) {
                        std::cout << "Time out" << std::endl;
                    }
                }
                if (maxMCS < turboIsoSlover.largestMapping.size()) {
                    maxMCS = turboIsoSlover.largestMapping.size();
                    bestId = dataGraph.familyIndex;
                    if (maxMCS == queryGraph.n) {
                        break;
                    }
                }                  
            }
            // int mcs = mcsSolver.solve(queryGraph, dataGraph).size();
            // if (maxMCS < mcs) {
            //     maxMCS = mcs;
            //     bestId = dataGraph.familyIndex;
            //     if (maxMCS == queryGraph.n) {
            //         break;
            //     }
            // }    
        }
        runTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - startTime).count()/1000000.0;
        std::cout << "Found " << bestId << " match with " << queryGraph.familyIndex << " with MCS " << maxMCS << std::endl;
        std::cout << "Take " << runTime << " s" << std::endl;
        if (bestId == queryGraph.familyIndex) {
            correctWithoutEmb += 1;
        }
        timeWithoutEmb += runTime; 

        float minDistance = std::numeric_limits<float>::max();
        int bestIndex = -1;
        std::priority_queue<std::pair<float, int>> distanceQueue;
        startTime = std::chrono::steady_clock::now();

        std::vector<float> queryVector = queryGraph.embeddingVector;
        for (int dataId = 0 ; dataId < dataGraphEmbeddings.size();  dataId ++) {
            std::vector<float> dataEmbedding = dataGraphEmbeddings[dataId];
            float distance = 0;
            for (int k = 0; k <queryVector.size();k++) {
                distance += (dataEmbedding[k] - queryEmbedding[k]) * (dataEmbedding[k] - queryEmbedding[k]);
            }
            if (distanceQueue.size() < arguments.k) {
                distanceQueue.push({distance, dataId});
            } else if (distance < distanceQueue.top().first) {
                distanceQueue.pop();
                distanceQueue.push({distance, dataId});
            }
        }  
        maxMCS = 0;
        bestId = -1;           
        std::stack<int> S;
        while(distanceQueue.size()>0) {
            int currIndex = distanceQueue.top().second;
            distanceQueue.pop();
            S.push(currIndex);
        }
        while(S.size()>0) {
            int currIndex = S.top();
            S.pop();
            Graph dataGraph = dataGraphVector[currIndex];
            std::map<int,int> tempMapNode;
            if (arguments.vf2) {
                if (queryGraph.n <= dataGraph.n) {
                    try {
                        subGraphSlover.getAllSubGraphs(dataGraph,queryGraph, 0, 1);
                    }
                    catch(std::runtime_error& e) {
                        std::cout << "Time out" << std::endl;
                    }
                } else {
                    try {
                        subGraphSlover.getAllSubGraphs(queryGraph,dataGraph, 0, 1);
                    }
                    catch(std::runtime_error& e) {
                        std::cout << "Time out" << std::endl;
                    }
                }
                if (maxMCS < subGraphSlover.largestMapping.size()) {
                    maxMCS = subGraphSlover.largestMapping.size();
                    bestId = dataGraph.familyIndex;
                    if (maxMCS == queryGraph.n) {
                        break;
                    }
                }                
            } else {
                if (queryGraph.n <= dataGraph.n) {
                    try {
                        turboIsoSlover.getAllSubGraphs(dataGraph,queryGraph, 0, 1);
                    }
                    catch(std::runtime_error& e) {
                        std::cout << "Time out" << std::endl;
                    }
                } else {
                    try {
                        turboIsoSlover.getAllSubGraphs(queryGraph,dataGraph, 0, 1);
                    }
                    catch(std::runtime_error& e) {
                        std::cout << "Time out" << std::endl;
                    }
                }
                if (maxMCS < turboIsoSlover.largestMapping.size()) {
                    maxMCS = turboIsoSlover.largestMapping.size();
                    bestId = dataGraph.familyIndex;
                    if (maxMCS == queryGraph.n) {
                        break;
                    }
                }                  
            }
            // int mcs = mcsSolver.solve(queryGraph, dataGraph).size();
            // if (maxMCS < mcs) {
            //     maxMCS = mcs;
            //     bestId = dataGraph.familyIndex;
            //     if (maxMCS == queryGraph.n) {
            //         break;
            //     }
            // }  
        }  
        runTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - startTime).count()/1000000.0;
        if (bestId == queryGraph.familyIndex) {
            correctWithEmb += 1;
        }
        std::cout << "Found " << bestId << " match with " << queryGraph.familyIndex << " with MCS " << maxMCS << std::endl;
        std::cout << "Take " << runTime << " s" << std::endl;
        timeWithEmb += runTime;  
        std::cout << "====================================" << std::endl;
    }
    std::cout << timeWithEmb << " " << correctWithEmb << std::endl;
    std::cout << timeWithoutEmb << " " << correctWithoutEmb << std::endl;
}
