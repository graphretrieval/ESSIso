#pragma once

#include<vector>
#include<set>
#include<map>

class PCM_Node {
public:
	std::set<int> descendent;
	/*
	* for each of its children, we keep a mapping list from the vertex-id of this graph to the vertex-ids of its children
	*/
	std::map<int, std::vector<std::vector<int>>> containmentRelationshipMappingLists;
	std::map<int, std::set<int>> containmentRelationshipMappingSet;

	std::vector<int> equivalentGraph;

	/*
	* The direct children are saved in a vector
	*/
	std::vector<int> children;
	std::vector<int> parent;

	int graphId;
	bool isGeneratedQueryGraph;
	bool isHidden;
	bool isVisited;

	int numberOfComputedParents;
	int numberOfComputedChildren;
	int sameHeightPriority;

	PCM_Node() {
	};

	PCM_Node(int pGraphId, bool pIsGeneratedQueryGraph) {
		graphId = pGraphId;
		isGeneratedQueryGraph = pIsGeneratedQueryGraph;
		isHidden = false;
		isVisited = false;
		numberOfComputedParents = 0;
		numberOfComputedChildren = 0;
		sameHeightPriority = 0;
	};

	~PCM_Node() {
		descendent = std::set<int>();
		containmentRelationshipMappingLists = std::map<int, std::vector<std::vector<int>>>();
		containmentRelationshipMappingSet = std::map<int, std::set<int>>();
		equivalentGraph = std::vector<int>();
		children = std::vector<int>();
		parent = std::vector<int>();
	}

};

