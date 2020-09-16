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
    {"vf2", 'v', 0, 0, "Use vf2"},
    {"use distance utility", 'd', 0, 0, "Use distance utility"},
    {"timeout", 't', "timeout", 0, "Specify a timeout (seconds)"},
    {"nresults", 'e', "nresults", 0, "Specify max number of results"},
    {"ncalls", 'a', "ncalls", 0, "Specify max number of calls"},

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
    unsigned long totalTime = 0;
    int numberofFound = 0;
    int numberofFound2 = 0;
    int hit = 0;
    set_default_arguments();
    argp_parse(&argp, argc, argv, 0, 0, 0);
    Graph dataGraph = readGraph(arguments.datagraph, arguments.directed,
    arguments.edge_labelled, arguments.vertex_labelled);
    dataGraph.buildData();
    std::vector<Graph> queryGraphVector;
    
    DIR* dirp = opendir(arguments.querydir);
    struct dirent * dp;
    std::string querydir(arguments.querydir);
    std::vector<std::string> filenames;
    while ((dp = readdir(dirp)) != NULL) {
        if (dp->d_name[0] == 'q' && endsWith(dp->d_name, "dimas")) {
            std::string filename(dp->d_name);
            filenames.push_back(querydir+filename);
        }
    }
    closedir(dirp);
    sort(filenames.begin(), filenames.end());
    double buildTime = 0;
    for (std::string filename : filenames) {
        char c_filename[filename.size() + 1];
	    strcpy(c_filename, filename.c_str());
        Graph queryGraph = readGraph(c_filename, arguments.directed,
        arguments.edge_labelled, arguments.vertex_labelled);
        std::chrono::steady_clock::time_point beginBuild = std::chrono::steady_clock::now();
        queryGraph.buildData();
        buildTime += (std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - beginBuild).count() + 8300);
        queryGraphVector.push_back(queryGraph);     
    }
    std::cout << "Got " << queryGraphVector.size() << " query graph(s)" << std::endl;

    std::vector<Graph> coldstartGraphVector;
    if (arguments.coldstartdir) {
        DIR* cdirp = opendir(arguments.coldstartdir);
        struct dirent * cdp;
        std::string cquerydir(arguments.coldstartdir);
        std::vector<std::string> cfilenames;
        while ((cdp = readdir(cdirp)) != NULL) {
            if (cdp->d_name[0] == 'q') {
                std::string filename(cdp->d_name);
                cfilenames.push_back(cquerydir+filename);
            }
        }
        closedir(cdirp);
        for (std::string filename : cfilenames) {
            char c_filename[filename.size() + 1];
            strcpy(c_filename, filename.c_str());
            Graph queryGraph = readGraph(c_filename, arguments.directed,
            arguments.edge_labelled, arguments.vertex_labelled);
            queryGraph.buildData();
            coldstartGraphVector.push_back(queryGraph);     
        }
        std::cout << "Got " << coldstartGraphVector.size() << " coldstart query graph(s)" << std::endl;        
    }

    if (coldstartGraphVector.size() > arguments.cache_size) {
        coldstartGraphVector.resize(arguments.cache_size);
    }

    SubGraphSearch subGraphSlover = SubGraphSearch();
    subGraphSlover.timeOut = arguments.timeout;
    TurboIso turboIsoSlover = TurboIso();
    turboIsoSlover.timeOut = arguments.timeout;

    double overHeadTime = 0;
    double totalScanTime = 0;

    Cache graphCache = Cache(arguments.cache_size, arguments.lru, arguments.lfu);
    int graphIndex = 0;

    for (Graph queryGraph : coldstartGraphVector) {
        graphIndex++;
        std::unordered_map<int, float> distanceMap;
        std::priority_queue<std::pair<float, int>> distanceQueue;
        // graphCache.scanCache(queryGraph, &bestIndex, &minDistance, &distanceMap);
        std::chrono::steady_clock::time_point sStart = std::chrono::steady_clock::now();
        graphCache.kScanCache(queryGraph, &distanceQueue, &distanceMap, 5);
        float distanceUtility = 0;
        while(distanceQueue.size()>0) {
            distanceUtility +=  distanceQueue.top().first;
            distanceQueue.pop();
        }
        std::vector<std::map<int,int>> resultsGraph;
        try {
            turboIsoSlover.timeOut = 10;
            turboIsoSlover.getAllSubGraphs(dataGraph, queryGraph, 0, arguments.maxResult);
        }
        catch(std::runtime_error& e) {
            std::cout << e.what() << std::endl;
        }
        resultsGraph = turboIsoSlover.allMappings;
        std::cout << "Found " << resultsGraph.size() << " results" << std::endl;
        if (resultsGraph.size()>0) {
            float t = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - sStart).count()/1000000.0;
            if (t>0.001) 
                t = 0.001;   
            if (arguments.distance)
                t = t*distanceUtility;
            graphCache.insert(queryGraph, resultsGraph, t, graphIndex, distanceMap);
        }
    }

    for (Graph queryGraph : queryGraphVector) {
        graphIndex++;
        std::cout << "Graph Index: " << graphIndex << std::endl;
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        if (arguments.useCache) {
            int cachedGraphIndexCase1 = -1;
            int cachedGraphIndexCase2 = -1;
            int cachedGraphIndexCase3 = -1;
            std::map<int,int> mapNode;
            float utility = 0;
            float s = 0;
            float t = 0;
            std::chrono::steady_clock::time_point sStart = std::chrono::steady_clock::now();
            Graph * bestElem;
            
            float minDistance = std::numeric_limits<float>::max();
            int bestIndex = -1;
            std::unordered_map<int, float> distanceMap;
            std::priority_queue<std::pair<float, int>> distanceQueue;
            // graphCache.kScanCache(queryGraph, &distanceQueue, &distanceMap, 5);

            float distanceUtility = 0;
            for (auto elem : graphCache.getCacheGraphs())  {
                std::map<int,int> tempMapNode;
                bool timedout = false;
                std::vector<std::map<int,int>> tempResults;
                // std::chrono::steady_clock::time_point mcsbegin = std::chrono::steady_clock::now();
                if (queryGraph.n <= elem.second.n) {
                    try {
                        subGraphSlover.timeOut = 0.0001;
                        // subGraphSlover.getAllSubGraphs(queryGraph, elem.second, 0, 1);
                        subGraphSlover.getAllSubGraphs(elem.second,queryGraph, 0, 1);
                    }
                    catch(std::runtime_error& e) {
                        std::cout << e.what() << std::endl;
                        timedout = true;
                    }
                    // std::cout << "MCS time: " << std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - mcsbegin).count() << "[ms]" << std::endl;
                    tempMapNode = subGraphSlover.largestMapping;
                    if (tempMapNode.size() ==elem.second.n && elem.second.n == queryGraph.n) {
                        cachedGraphIndexCase1 = elem.first;
                        mapNode = tempMapNode;
                        break;
                    } else if (tempMapNode.size() == queryGraph.n && elem.second.n > queryGraph.n) {
                        cachedGraphIndexCase3 = elem.first;
                        mapNode = tempMapNode;
                        break;
                    } 
                    else if (arguments.case2 && tempMapNode.size() > mapNode.size() && tempMapNode.size() +2 > queryGraph.n && tempMapNode.size()< queryGraph.n) {
                        mapNode = tempMapNode;
                        cachedGraphIndexCase2 =elem.first;
                        break;
                    } 
                } 
                else if (arguments.case2) {
                    try {
                        subGraphSlover.timeOut = 0.0001;
                        subGraphSlover.getAllSubGraphs(queryGraph, elem.second, 0, 1);
                        // subGraphSlover.getAllSubGraphs(*bestElem,queryGraph, 0, 1);
                    }
                    catch(std::runtime_error& e) {
                        std::cout << e.what() << std::endl;
                        timedout = true;
                    }
                    // std::cout << "MCS time: " << std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - mcsbegin).count() << "[ms]" << std::endl;
                    tempMapNode = subGraphSlover.largestMapping;
                    if (tempMapNode.size() > mapNode.size() && tempMapNode.size() +2 > queryGraph.n && tempMapNode.size()< queryGraph.n) {
                        mapNode.clear();
                        for (auto i : tempMapNode) 
                            mapNode.insert({i.second, i.first});
                        cachedGraphIndexCase2 =elem.first;
                        break;
                    } 
                }
            }
            totalScanTime += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - sStart).count()/1000.0;

            bool useCache = false;
            if (cachedGraphIndexCase1 != -1) {
                hit ++;
                std::cout << "Found case 1:" << cachedGraphIndexCase1 << std::endl;
                std::vector<std::map<int,int>> * cachedEmbedding = graphCache.getEmbeddingsById(cachedGraphIndexCase1); 
                if (cachedEmbedding != NULL) {
                    numberofFound ++;
                    numberofFound2 += cachedEmbedding->size();
                    useCache = true;
                }
                s = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - sStart).count()/1000000.0;
                graphCache.updateCacheHit(cachedGraphIndexCase1, s, graphIndex);
            } else if (cachedGraphIndexCase3 != -1) {
                hit ++;
                std::cout << "Found case 3:" << cachedGraphIndexCase3 << std::endl;
                std::vector<std::map<int,int>> * cachedEmbedding = graphCache.getEmbeddingsById(cachedGraphIndexCase3); 
                if (cachedEmbedding != NULL) {
                    numberofFound ++;
                    numberofFound2 += cachedEmbedding->size();
                    useCache = true;               
                }
                s = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - sStart).count()/1000000.0;
                graphCache.updateCacheHit(cachedGraphIndexCase3, s, graphIndex);
            } else if (cachedGraphIndexCase2 != -1) {
                std::vector<std::map<int,int>> * cachedEmbedding = graphCache.getEmbeddingsById(cachedGraphIndexCase2); 
                if (cachedEmbedding != NULL) {
                    hit ++;
                    std::cout << "Found case 2:" << cachedGraphIndexCase2 << std::endl;
                    useCache = true;
                    std::set<std::map<int,int>> candidateBigU =  ProjectEmbedding::crossProjectEmbedding(* cachedEmbedding, mapNode);
                    std::vector<std::map<int,int>> resultsGraph;
                    if (arguments.vf2) {
                        try {
                            subGraphSlover.timeOut = arguments.timeout;
                            subGraphSlover.getAllSubGraphs(dataGraph, queryGraph, 0, arguments.maxResult);
                        }
                        catch(std::runtime_error& e) {
                            std::cout << e.what() << std::endl;
                        }
                        resultsGraph = subGraphSlover.allMappings;
                    } else {
                        try {
                            turboIsoSlover.timeOut = arguments.timeout;
                            turboIsoSlover.getAllSubGraphs(dataGraph, queryGraph, 0, arguments.maxResult);
                        }
                        catch(std::runtime_error& e) {
                            std::cout << e.what() << std::endl;
                        }
                        resultsGraph = turboIsoSlover.allMappings;
                    }
                    std::cout << "Found " << resultsGraph.size() << " results" << std::endl;
                    if (resultsGraph.size()>0) {
                        if (graphCache.isLRU || graphCache.isLFU) {
                            std::cout << "add query graph " << graphIndex << " to cache" << std::endl; 
                            graphCache.insert(queryGraph, resultsGraph, t, graphIndex, distanceMap);       
                        } else {
                            t = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - sStart).count()/1000000.0;
                            if (arguments.distance)
                                t = t*distanceUtility;
                            if (t>=graphCache.minUtility) {
                                std::cout << "add query graph " << graphIndex << " to cache" << std::endl; 
                                graphCache.insert(queryGraph, resultsGraph, t, graphIndex, distanceMap);
                            }   
                        }
                        numberofFound2 +=  resultsGraph.size();
                        numberofFound ++;  
                    }
                    s = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - sStart).count()/1000000.0;
                graphCache.updateCacheHit(cachedGraphIndexCase2, s, graphIndex);
                } 
            } 
            overHeadTime += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - sStart).count();
            if (!useCache) {
                std::cout << "Find from the beginning" << std::endl;
                bool timedout = false;
                std::vector<std::map<int,int>> resultsGraph;
                if (arguments.vf2) {
                    try {
                        subGraphSlover.timeOut = arguments.timeout;
                        subGraphSlover.getAllSubGraphs(dataGraph, queryGraph, 0, arguments.maxResult);
                    }
                    catch(std::runtime_error& e) {
                        std::cout << e.what() << std::endl;
                    }
                    resultsGraph = subGraphSlover.allMappings;
                } else {
                    try {
                        turboIsoSlover.timeOut = arguments.timeout;
                        turboIsoSlover.getAllSubGraphs(dataGraph, queryGraph, 0, arguments.maxResult);
                    }
                    catch(std::runtime_error& e) {
                        std::cout << e.what() << std::endl;
                    }
                    resultsGraph = turboIsoSlover.allMappings;
                }
                std::cout << "Found " << resultsGraph.size() << " results" << std::endl;
                if (resultsGraph.size()>0) {
                    std::chrono::steady_clock::time_point sStart2 = std::chrono::steady_clock::now();
                    if (graphCache.isLRU || graphCache.isLFU) {
                        std::cout << "add query graph " << graphIndex << " to cache" << std::endl; 
                        graphCache.insert(queryGraph, resultsGraph, t, graphIndex, distanceMap);       
                    } else {
                        t = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - sStart).count()/1000000.0;
                        if (arguments.distance)
                            t = t*distanceUtility;
                        if (t>=graphCache.minUtility) {
                            std::cout << "add query graph " << graphIndex << " to cache" << std::endl; 
                            graphCache.insert(queryGraph, resultsGraph, t, graphIndex, distanceMap);
                        }   
                    }
                    numberofFound2 +=  resultsGraph.size();
                    numberofFound ++;  
                    overHeadTime += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - sStart2).count();                      
                }
            }
        } else {
            bool timedout = false;
            std::vector<std::map<int,int>> resultsGraph;
            if (arguments.vf2) {
                try {
                    subGraphSlover.timeOut = arguments.timeout;
                    subGraphSlover.getAllSubGraphs(dataGraph, queryGraph, 0, arguments.maxResult);
                }
                catch(std::runtime_error& e) {
                    std::cout << e.what() << std::endl;
                }
                resultsGraph = subGraphSlover.allMappings;
            } else {
                try {
                    turboIsoSlover.timeOut = arguments.timeout;
                    turboIsoSlover.getAllSubGraphs(dataGraph, queryGraph, 0, arguments.maxResult);
                }
                catch(std::runtime_error& e) {
                    std::cout << e.what() << std::endl;
                }
                resultsGraph = turboIsoSlover.allMappings;
            }
            std::cout << "Found " << resultsGraph.size() << " results" << std::endl;
            if (resultsGraph.size()>0) {
                std::cout << "add query graph " << graphIndex << " to cache" << std::endl; 
                numberofFound2 +=  resultsGraph.size();
                numberofFound ++;
            }
        }
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        int runningTime = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()/1000.0;
        totalTime +=  runningTime;
        // std::cout << "Utility" << std::endl;
        // for (auto i : graphCache.utilities) {
        //     std::cout << i.first << ":" << i.second << std::endl;
        // }
        // std::cout << "next evited " << graphCache.nextIndexToEvicted << ":" << graphCache.minUtility << std::endl;
        std::cout << "Running time: " << runningTime << "[ms]" << std::endl;
        std::cout << "======================================" << std::endl;
    }
    std::cout << "Total build time: " << buildTime/1000 << std::endl;
    std::cout << "Total scan time: " << totalScanTime << std::endl;
    std::cout << "Total overhead time: " << overHeadTime/1000 << std::endl;
    std::cout << "Total running time: " << totalTime << std::endl;
    std::cout << "Number of found: " << numberofFound << " " << numberofFound2 << std::endl;
    std::cout << "Hit: " << hit << std::endl;
}
