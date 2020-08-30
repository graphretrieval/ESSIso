#include <map>
#include <set>
#include <iostream>
#include <math.h> 
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "SubGraphSearch.hpp"
#include "Graph.hpp"
#include "Utils.hpp"

using namespace std;

vector<map<int,int>> SubGraphSearch::getAllSubGraphs(const Graph & pdataGraph, const Graph &pqueryGraph, int maxRCall_, int maxEmbedding_) {
	startTime = std::chrono::steady_clock::now();
    dataGraph = pdataGraph;
    queryGraph = pqueryGraph;
	maxEmbedding = maxEmbedding_;
	maxRCall = maxRCall_;
	nCall = 0;
	needReturn = false;
	allMappings = vector<map<int,int>>();
	mapping = map<int,int>();
	reverseMapping = map<int,int>();
	largestMapping.clear();
	
    filterCandidates();

    if (candidates.size() == queryGraph.n)
		isomorphismSearch();

	return allMappings;
}

vector<map<int,int>> SubGraphSearch::getAllSubGraphsWrapper(const Graph & pdataGraph, const Graph &pqueryGraph, int maxRCall_, int maxEmbedding_, int timeout) {
	std::mutex m;
    std::condition_variable cv;
    std::vector<std::map<int,int>> retValue;

    std::thread t([&cv, &retValue, &pdataGraph, &pqueryGraph, &maxRCall_, &maxEmbedding_, this]() 
    {
        retValue = getAllSubGraphs(pdataGraph,pqueryGraph, maxRCall_, maxEmbedding_);
        cv.notify_one();
    });

    t.detach();

    {
        std::unique_lock<std::mutex> l(m);
        if(cv.wait_for(l, std::chrono::seconds(timeout)) == std::cv_status::timeout) {
			throw std::runtime_error("Timeout");
		}      
    }
    return retValue;
}


vector<map<int,int>> SubGraphSearch::getAllSubGraphsCase2(const Graph & pdataGraph, const Graph &pqueryGraph, set<map<int,int>> candidateBigU, int maxRCall_, int maxEmbedding_) {
    // std::cout << "BEGIN getAllSubGraphsCase2" << std::endl;
	startTime = std::chrono::steady_clock::now();
	dataGraph = pdataGraph;
    queryGraph = pqueryGraph;
	maxEmbedding = maxEmbedding_;
	maxRCall = maxRCall_;
	nCall = 0;
	needReturn = false;
	allMappings = vector<map<int,int>>();
	largestMapping.clear();

    filterCandidates();

    if (candidates.size() == queryGraph.n) {
		for (auto it = candidateBigU.begin(); it != candidateBigU.end(); it++) {
			if (needReturn)
				break;
			mapping = *it;
			reverseMapping = map<int,int>();
			for (auto it2 = mapping.begin(); it2 != mapping.end(); it2++) 
				reverseMapping[it2->second] = it2->first;
			isomorphismSearch();
		}
	}
	
	return allMappings;
}

vector<map<int,int>> SubGraphSearch::getAllSubGraphsCase3(const Graph & pdataGraph, const Graph &pqueryGraph, set<map<int,int>> candidateBigU) {
    dataGraph = pdataGraph;
    queryGraph = pqueryGraph;
	allMappings = vector<map<int,int>>();

    filterCandidates();

	// C(u) / project(u)(R(wi))
	for (auto it = candidateBigU.begin(); it != candidateBigU.end(); it++) {
		for (auto it2 = it->begin(); it2 != it->end(); it2++) 
			candidates[it2->first].erase(it2->second);
	}

	vector<int> mustIn = vector<int>();
	vector<int> nonMustIn = vector<int>();
	for (auto it = candidates.begin(); it != candidates.end(); it++) {
		if (it->second.size()==0) mustIn.push_back(it->first);
		else nonMustIn.push_back(it->first);
	}

    if (candidates.size() == queryGraph.n) {
		// for subset2 in power set of non Must In {
		for (int counter = 0; counter < pow(2, nonMustIn.size()); counter++) {
			// Vw = all node - subset2
			set<int> Vw = set<int>(mustIn.begin(), mustIn.end());

			for(int i=0; i < nonMustIn.size(); i++) 
				if(counter & (1 << i)) Vw.insert(nonMustIn[i]);

		    //  get subembedding of subset1 from candidateBigU
			set<map<int,int>> subCandidateBigU = ProjectEmbedding::projectEmbedding(candidateBigU, Vw);

			for (auto it = subCandidateBigU.begin(); it != subCandidateBigU.end(); it++) {
				mapping = *it;
				reverseMapping = map<int,int>();
				for (auto it2 = mapping.begin(); it2 != mapping.end(); it2++) 
					reverseMapping[it2->second] = it2->first;
				isomorphismSearch();
			}
		}
	}
	
	return allMappings;
}


void SubGraphSearch::showEmbedding() {
	cout << "Found " << allMappings.size() << " mapping" << endl;
    for (vector<map<int,int>>::iterator it = allMappings.begin(); it != allMappings.end(); it ++) {
        for (map<int,int>::iterator it2 = it->begin(); it2 != it->end(); it2++) {
            cout << it2->first << ":" << it2->second << " ";
        }
        cout << endl;
    }
}
void SubGraphSearch::filterCandidates() {
    candidates = std::map<int, std::set<int>>();
    // std::cout << "Candidates" << endl;
    for (int nodeIndex = 0; nodeIndex < queryGraph.n; nodeIndex++) {
		// cout << nodeIndex << " " << queryGraph.label[nodeIndex] << endl;
		map<int,set<int>>::iterator it = dataGraph.labelVertexList.find(queryGraph.label[nodeIndex]);
		if (it != dataGraph.labelVertexList.end()) {
			candidates.insert({nodeIndex, it->second});
		}
	}
	// for (std::map<int, std::set<int>>::iterator it = candidates.begin(); it != candidates.end();it++) {
	// 	std::cout << it->first << endl;
	// 	for (std::set<int>::iterator it2 = it->second.begin(); it2 != it->second.end();it2++) 
	// 		std::cout << *it2 << " ";
	// 	std::cout << endl;
	// }
}

int SubGraphSearch::nextQueryVertex() {
	for (vector<int>::iterator it = queryGraph.DFSTraversalOrder.begin(); it != queryGraph.DFSTraversalOrder.end(); it++) {
		if (mapping.find(*it) == mapping.end()) {
			return *it;
		}
	}
}

bool SubGraphSearch::refineCandidates(int u, int v) {
	if (reverseMapping.find(v) != reverseMapping.end())
		return false;
	//TODO: Re-write degree filter
	// int degU = queryGraph.degVertexList.find(u)->second;
	// int degV = dataGraph.degVertexList.find(v)->second;
	// return degU <= degV;
	if (!degreeFilter(u,v)) return false;
	return true;
}

bool SubGraphSearch::degreeFilter(int u, int v) {
	for (std::map<int, std::vector<int>>::iterator queryVertexLabelVertexIt = queryGraph.vertexlabelVertexList[u].begin(); 
		queryVertexLabelVertexIt != queryGraph.vertexlabelVertexList[u].end(); queryVertexLabelVertexIt++ ) {
		if (dataGraph.vertexlabelVertexList[v].find(queryVertexLabelVertexIt->first) ==dataGraph.vertexlabelVertexList[v].end() )
			return false;
		else if (queryVertexLabelVertexIt->second.size() > dataGraph.vertexlabelVertexList[v].find(queryVertexLabelVertexIt->first)->second.size())
			return false;
	}
	return true;
}

bool SubGraphSearch::isJoinable(int u, int v) {
	for (set<int>::iterator neighNodeU = queryGraph.adjList[u].begin(); neighNodeU != queryGraph.adjList[u].end(); neighNodeU++) {
		map<int,int>::iterator it = mapping.find(*neighNodeU);
		if (it == mapping.end()) {
			continue;
		} else {
			// if (dataGraph.adjmat[v][it->second]==1)
			if (dataGraph.adjList[v].find(it->second)!=dataGraph.adjList[v].end())
				continue;
			else
				return false;
		}
	}
	return true;
}

void SubGraphSearch::updateState(int u, int v) {
	// std::cout << "UPDATE STATE" << std::endl;
	mapping.insert({u, v});
	if (mapping.size() > largestMapping.size())
		largestMapping = mapping;
	// std::cout << mapping.size() << std::endl;
	reverseMapping.insert({v,u});
}

void SubGraphSearch::restoreState(int u, int v) {
	mapping.erase(u);
	reverseMapping.erase(v);
}

void SubGraphSearch::isomorphismSearch() {

	std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();

	if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() >= timeOut*1000) 
		throw std::runtime_error("Timeout");

	nCall++;
	if ((nCall >= maxRCall) && maxRCall>0)  {
		needReturn = true;
		return;
	}
	
	if (needReturn)
		return;
	// for (map<int,int>::iterator it = mapping.begin(); it != mapping.end(); it ++) {
    //     cout << it->first << ":" << it->second << ", ";
    //     cout << endl;
    // }
	if (mapping.size() == queryGraph.n) {
		map<int,int> tempMapping = mapping;
		allMappings.push_back(tempMapping);
		if (allMappings.size() >= maxEmbedding)
			needReturn = true;
		// cout << "found one\n";
		return;
	}
	int u = nextQueryVertex();
	set<int> candidates_u = candidates.find(u)->second;

	for (set<int>::iterator v = candidates_u.begin(); v != candidates_u.end(); v++) {
		if (needReturn)
			return;
		if (!refineCandidates(u, *v)) {
			continue;
		}
		// cout << "Test " << u << " " << *v << endl;
		if (isJoinable(u, *v)) {
			// cout << "Join " << u << " with " << *v << endl;
			updateState(u, *v);
			isomorphismSearch();
			restoreState(u, *v);
		} 
		// else {
		// 	cout << "Can't join " << u << " with " << *v << endl;
		// }
	}
}





