#ifndef ADJACENCE_LISTS_GRAPH_IO
#define ADJACENCE_LISTS_GRAPH_IO

#include"AdjacenceListsGraph.h"
#include<fstream>
#include<vector>


class AdjacenceListsGRAPH_IO{
public:
	static void show(const AdjacenceListsGRAPH*);
	static void loadGraphFromFile(std::ifstream & graphFile, std::vector<AdjacenceListsGRAPH> & graphList, bool isDataGraph);
};

#endif