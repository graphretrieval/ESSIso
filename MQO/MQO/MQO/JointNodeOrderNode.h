#pragma once

#include<vector>
#include<map>
#include<set>

using namespace std;

class JointNodeOrderNode {
public:
	int jointNodeId;

	// for each parent vertex, it may covered a same query vertex as other parents
	std::vector<int> preMultiCoveredVertex;

	// for each parent vertex, it may be connected to some verties already matched
	std::vector<std::vector<int>> preConnectedMappedVertex;


	// within this parent mapping, there is some uncoveredEdges
	// the first int represents the combo node id of the uncovered query vertex 
	std::vector<std::map<int, std::vector<int>>> uncoveredEdges;

};