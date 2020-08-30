#pragma once
#ifndef FIND_MAXIMAL_DISJOINT_CONN_CLIQUES_H
#define FIND_MAXIMAL_DISJOINT_CONN_CLIQUES_H


#include<vector>
#include<set>

/*
* Based on the FindMaximumClique computed a maximum disconnected clique forest,
* we split the forest into seperated cliques each of whom is connected
* in the seperated cliques, all these separated clique graphs. they are disjointed
* @return std::vector<std::vector<int>> & similiarQueryGroups
*/
class FindMaximalDisjointConnCliques {
public:
	static void findMaximalDisjointConnCliques(bool ** graphMatrix, int graphSize, std::vector<std::vector<int>> & similiarQueryGroups);
};

#endif