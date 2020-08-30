#pragma once
#ifndef FREQUENT_PATTERN_GROUP_H
#define FREQUENT_PATTERN_GROUP_H


#include<vector>
#include"AdjacenceListsGraph.h"


using namespace std;

class FrequentPatternGroup {

public:
	void frequentPatternGroup(std::vector<AdjacenceListsGRAPH> & queryGraphVector, std::vector<std::vector<int>> & similiarQueryGroups);

};

#endif