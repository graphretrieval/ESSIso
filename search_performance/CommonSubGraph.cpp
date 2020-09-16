#include <map>
#include <vector>
#include <set>
#include <algorithm> 
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "Graph.hpp"
#include "CommonSubGraph.hpp"

biNodeSet::biNodeSet() {
    setG = std::set<int>();
    setH = std::set<int>();
}

biNodeSet::biNodeSet(std::set<int> setG_, std::set<int> setH_) {
    setG = setG_;
    setH = setH_;
}

int MCS::calBound(std::map<std::string, biNodeSet> future){
    int bound = 0;
    for (std::map<std::string, biNodeSet>::iterator it = future.begin(); it != future.end(); it++) {
        bound += std::min(it->second.setG.size(), it->second.setH.size());
    }
    return bound;
}

std::pair<std::string, biNodeSet>  MCS::selectLabelClass(std::map<std::string, biNodeSet> future) {
    std::pair<std::string, biNodeSet> selectedPair = {"-1", {biNodeSet()}};
    int minValue = std::max(g.n, h.n);
    for (std::map<std::string, biNodeSet>::iterator pair = future.begin();pair != future.end();pair++) {
        std::string binaryLabel = pair->first.substr(pair->first.find("-")+1); 
        if ((binaryLabel.find("1") == std::string::npos) && (binaryLabel.length()>0)) 
            continue;
        int tempValue = std::max(pair->second.setG.size(), pair->second.setH.size());
        if (tempValue < minValue) {
            selectedPair = *pair; 
            minValue = tempValue;
        }
    }
    return selectedPair;
}

int MCS::selectVertex(std::set<int> setG, const Graph & graph) {
    std::set<int>::iterator selectedVertex = setG.begin();
    for (std::set<int>::iterator vertex = setG.begin();vertex != setG.end();vertex++) {
        if (graph.degVertexList.find(*selectedVertex)->second < graph.degVertexList.find(*vertex)->second) 
            selectedVertex = vertex; 
    }
    return *selectedVertex;
}

std::map<std::string, biNodeSet> MCS::createOriginFuture() {
    std::map<std::string, biNodeSet> future = std::map<std::string, biNodeSet>();
    std::map<int, std::set<int>>::iterator itH;
    for (std::map<int, std::set<int>>::iterator itG = g.labelVertexList.begin(); itG != g.labelVertexList.end(); itG++ ) {
        std::string key = std::to_string(itG->first) + "-";
        itH = h.labelVertexList.find(itG->first);
        if (itH == h.labelVertexList.end())
            continue;
        else 
            future.insert({key, {itG->second, itH->second} });
    }
    return future;
}

std::map<int,int> MCS::solve(Graph & g_, Graph & h_) {
    g = g_;
    h = h_;
    incumbent.clear();
    std::map<std::string, biNodeSet> future = createOriginFuture();
    search(future);
    return incumbent;
}


std::map<int,int> MCS::solveWrapper(Graph & g_, Graph & h_) {
    std::mutex m;
    std::condition_variable cv;
    std::map<int,int> retValue;

    std::thread t([&cv, &retValue, &g_, &h_, this]() 
    {
        retValue = solve(g_,h_);
        cv.notify_one();
    });

    t.detach();

    {
        std::unique_lock<std::mutex> l(m);
        if(cv.wait_for(l, std::chrono::seconds(4)) == std::cv_status::timeout) 
            throw std::runtime_error("Timeout");
    }

    return retValue;
}

void MCS::search(std::map<std::string, biNodeSet> future) {
    if (mapping.size() > incumbent.size()) {
        incumbent = mapping;
    }
    int bound = mapping.size() + calBound(future);
    if (bound <= incumbent.size()) return;
    if (future.size() == 0) return; 

    std::pair<std::string, biNodeSet> selectedPair = selectLabelClass(future);
    if (selectedPair.first == "-1") return;
    int v = selectVertex(selectedPair.second.setG, g);

    for (std::set<int>::iterator w = selectedPair.second.setH.begin(); w != selectedPair.second.setH.end(); w++) {
        std::map<std::string, biNodeSet> newFuture = std::map<std::string, biNodeSet>();
        // construct new future
        for (std::map<std::string, biNodeSet>::iterator pair = future.begin();pair != future.end();pair++) {
            std::set<int> setG1;
            std::set<int> setH1;
            set_intersection(pair->second.setG.begin(),pair->second.setG.end(),g.adjList[v].begin(),g.adjList[v].end(),
                                std::inserter(setG1,setG1.begin()));
            set_intersection(pair->second.setG.begin(),pair->second.setG.end(),h.adjList[*w].begin(),h.adjList[*w].end(),
                                std::inserter(setH1,setH1.begin()));
            setG1.erase(v);
            setH1.erase(*w);

            if (setG1.size() > 0 && setH1.size() > 0) {
                newFuture.insert({pair->first+"1", {setG1, setH1}});
            }

            std::set<int> setG0;
            std::set<int> setH0;
            set_intersection(pair->second.setG.begin(),pair->second.setG.end(),g.nonadjList[v].begin(),g.nonadjList[v].end(),
                                std::inserter(setG0,setG0.begin()));
            set_intersection(pair->second.setG.begin(),pair->second.setG.end(),h.nonadjList[*w].begin(),h.nonadjList[*w].end(),
                                std::inserter(setH0,setH0.begin()));

            setG0.erase(v);
            setH0.erase(*w);
        } 
        mapping.insert({v,*w});
        search(newFuture);
        mapping.erase(v);
    }
    future[selectedPair.first].setG.erase(v);
    if (future[selectedPair.first].setG.size()==0) {
        future.erase(selectedPair.first);
    }
    search(future);
}

void MCS::showEmbedding() {
    for(auto it = incumbent.begin(); it != incumbent.end(); it++) {
        std::cout << it->first << ":" << it->second << " ";
    }
    std::cout << std::endl;
}