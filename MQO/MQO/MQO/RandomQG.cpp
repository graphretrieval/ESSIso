#include "RandomQG.h"
#include"AdjacenceListsGraph.h"
#include<random>
#include<time.h>

using namespace std;



/*
* A pure random query generator. 
* Given a data graph, will generate random graphs with random connetion and random labels from the label set of the data graph
*/
RandomQG::RandomQG() {}

RandomQG::RandomQG(AdjacenceListsGRAPH * pDataGraph, int pQueryType, std::ofstream * pResultFile, int pNumberOfGraphs)
{
	queryType = pQueryType;
	resultFile = pResultFile;


	generator = default_random_engine(time(NULL));

	/* Default configuration values */
	dataGraph = pDataGraph;
	numberOfGraphs = pNumberOfGraphs;

	BASE_QUERY_SIZE = 10;
	MAXIMUM_QUERY_SIZE = 15;
}

RandomQG::~RandomQG()
{}

void RandomQG::generateQueries()
{

	if (queryType == 0) {
		generateSubgraphs();
	}

}

void RandomQG::generateSubgraphs() {

	std::uniform_int_distribution<int> distribution_query_size(BASE_QUERY_SIZE, MAXIMUM_QUERY_SIZE);
	int querySize = distribution_query_size(generator);

	// random generate a query graph
	std::uniform_int_distribution<int> distribution_label(0, dataGraph->getLabelSet()->size() - 1); // generates number in the range 1...numberOfLabels
	std::uniform_int_distribution<int> distribution_vertex(0, querySize - 1); // generates number in the range 1...numberOfLabels
	int dice_roll;
	set<int> vertexLabelSet;
	set<int> vertexIdSet;
	set<std::pair<int, int>> edgeSet;

	// for each graph
	for (int indexGraph = 0; indexGraph < numberOfGraphs; indexGraph++) {
		(*resultFile) << "t " << " # " << indexGraph << "\n";
		vertexIdSet = std::set<int>();
		vertexLabelSet = std::set<int>();
		edgeSet = set<std::pair<int, int>>();

		//generate vertices
		for (int indexVertex = 0; indexVertex < querySize; indexVertex++) {

			// we tend to generate the graph vertices with a different label
			std::set<int>::iterator labelIterator = dataGraph->getLabelSet()->begin();
			std::advance(labelIterator, distribution_label(generator));
			dice_roll = *labelIterator;

			while (vertexLabelSet.find(dice_roll) != vertexLabelSet.end()) {
				dice_roll = distribution_label(generator);
			}
			vertexLabelSet.insert(dice_roll);
			if (vertexLabelSet.size() == dataGraph->getLabelSet()->size()) {
				vertexLabelSet = set<int>();
			}
			(*resultFile) << "v " << indexVertex << " " << dice_roll << "\n";
		}
		//generate edges
		// the number of edges = n-1;
		for (int indexEdge = 0; indexEdge < querySize - 1; indexEdge++) {
			int startVertex, endVertex;
			do {
				startVertex = distribution_vertex(generator);
				endVertex = distribution_vertex(generator);
				if (startVertex > endVertex) {
					swap(startVertex, endVertex);
				}
				else if (startVertex == endVertex) {
					continue;
				}
				if (vertexIdSet.find(startVertex) != vertexIdSet.end() && vertexIdSet.find(endVertex) != vertexIdSet.end()) {
					continue;
				}
				else if (indexEdge > 0 && vertexIdSet.find(startVertex) == vertexIdSet.end() && vertexIdSet.find(endVertex) == vertexIdSet.end()) {
					continue;
				}
				else {
					break;
				}
			} while (1);

			vertexIdSet.insert(startVertex);
			vertexIdSet.insert(endVertex);
			edgeSet.insert(std::pair<int, int>(startVertex, endVertex));
		}
		

		for (set<std::pair<int, int>>::iterator edgeIterator = edgeSet.begin(); edgeIterator != edgeSet.end(); edgeIterator++) {
			(*resultFile) << "e " << edgeIterator->first << " " << edgeIterator->second << " 0" << "\n";
		}
	}
}

