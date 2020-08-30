#pragma once

#include<vector>
#include"AdjacenceListsGraph.h"

using namespace std;

class JointGraphNode {
public:
	/*
	 * the pcmId of its corresponding parent in the PCM
	 */
	int parentId;

	/*
	 * if the joint node represents a parent. the mapping contains a embedding from its parent to this query graph
	 * if the joint node represents a stand along vertex, this query vertex will be put into the mapping
	 */
	std::vector<int> mapping;

	JointGraphNode() {
		parentId = -1;
	}
};

class JointGraph {
public:

	std::vector<JointGraphNode> jointNodes;
	// the int is the node index, not the parent id.
	std::map<int, std::vector<int>> jointEdges;

public:
	void clear() {
		jointNodes = std::vector<JointGraphNode>();
		jointEdges = std::map<int, std::vector<int>>();
	}

	bool edge(int from, int to) {
		std::map<int, std::vector<int>>::iterator edgeIter = jointEdges.find(from);
		if (edgeIter != jointEdges.end()) {
			for (vector<int>::iterator adjIter = edgeIter->second.begin(); adjIter != edgeIter->second.end(); adjIter++) {
				if (*adjIter == to) {
					return true;
				}
			}
		}
		return false;
	}

	void insertEdge(int from, int to) {
		std::map<int, std::vector<int>>::iterator edgeItee1 = jointEdges.find(from);
		std::map<int, std::vector<int>>::iterator edgeItee2 = jointEdges.find(to);

		if (edgeItee1 == jointEdges.end()) {
			std::vector<int> adj;
			adj.push_back(to);
			jointEdges.insert(std::pair<int, std::vector<int>>(from, adj));
		}
		else {
			edgeItee1->second.push_back(to);
		}
		if (edgeItee2 == jointEdges.end()) {
			std::vector<int> adj;
			adj.push_back(from);
			jointEdges.insert(std::pair<int, std::vector<int>>(to, adj));
		}
		else {
			edgeItee2->second.push_back(from);
		}
	}


};

