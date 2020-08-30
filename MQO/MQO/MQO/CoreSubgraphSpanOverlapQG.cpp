#include "CoreSubgraphSpanOverlapQG.h"
#include"AdjacenceListsGraph.h"
#include<random>
#include<time.h>

using namespace std;

/*
* We first find a base subgraph and then find a set of larger graphs that contains this base graph
* We can specify how many graphs for each core-group. This version hasn't use any strategy to distrubute the labels.
* It is suits for data graphs with very few labels but with large degrees
*/

CoreSubgraphSpanOverlapQG::CoreSubgraphSpanOverlapQG() {}

CoreSubgraphSpanOverlapQG::CoreSubgraphSpanOverlapQG(AdjacenceListsGRAPH * pDataGraph, int pQueryType, std::ofstream * pResultFile, int pNumberOfCores, int pQueryGroupSize)
{
	dataGraph = pDataGraph;
	queryType = pQueryType;
	resultFile = pResultFile;

	NUMBER_CORES = pNumberOfCores;

	dataGraph->buildVertexLabelVertexList();

	generator = default_random_engine(time(NULL));

	queryGraphIndex = 0;


	BASE_QUERY_SIZE = 10;
	MAXIMUM_QUERY_SIZE = 15;
	/*
	 * Total number of queries will be groupSize * numberOfCores + duplications
	 */
	QUERY_GROUP_SIZE = pQueryGroupSize;
}

CoreSubgraphSpanOverlapQG::~CoreSubgraphSpanOverlapQG()
{}

void CoreSubgraphSpanOverlapQG::generateQueries()
{

	set<int> coreVerticesSet;
	vector<int> coreVerticesVector;
	std::uniform_int_distribution<int> distribution_vertex_id(0, dataGraph -> getNumberOfVertexes());

	while (coreVerticesSet.size() < NUMBER_CORES) {
		int coreVertexId = distribution_vertex_id(generator);
		if (dataGraph->getVertexAddressByVertexId(coreVertexId)->inDegree > MAXIMUM_QUERY_SIZE) {
			if (coreVerticesSet.insert(coreVertexId).second == true) {
				coreVerticesVector.push_back(coreVertexId);
			}
		}
	}

	if (queryType == 0) {
		generateSubgraphs(coreVerticesVector);
	}
	else {
		generatePaths(coreVerticesVector);
	}

}

void CoreSubgraphSpanOverlapQG::generateSubgraphs(std::vector<int> & coreVerticesVector) {


	for (size_t coreIndex = 0; coreIndex < coreVerticesVector.size(); coreIndex++) {

		std::vector<int> baseVertexLabelVector;
		std::vector<int> baseVertexIdVector;

		std::set<int> baseVertexIdSet;


		AdjacenceListsGRAPH::Vertex * attachVertex = dataGraph->getVertexAddressByVertexId(coreVerticesVector[coreIndex]);

		baseVertexIdSet.insert(coreVerticesVector[coreIndex]);
		baseVertexIdVector.push_back(coreVerticesVector[coreIndex]);
		baseVertexLabelVector.push_back(attachVertex->label);


		while (baseVertexIdVector.size() < BASE_QUERY_SIZE) {

			std::uniform_int_distribution<int> distribution_vertex_next_id(0, baseVertexIdVector.size() - 1);
			attachVertex = dataGraph->getVertexAddressByVertexId(baseVertexIdVector[distribution_vertex_next_id(generator)]);
			std::uniform_int_distribution<int> distribution_vertex_label(0, attachVertex->labelVertexList.size() - 1);

			std::map<int, vector<int>>::iterator labelVertexIterator = attachVertex->labelVertexList.begin();
			std::advance(labelVertexIterator, distribution_vertex_label(generator));

			std::uniform_int_distribution<int> distribution_vertex_id(0, labelVertexIterator->second.size() - 1);
			int neighbourIndex = distribution_vertex_id(generator);
			if (baseVertexIdSet.insert(labelVertexIterator->second.at(neighbourIndex)).second == true) {

				baseVertexIdVector.push_back(labelVertexIterator->second.at(neighbourIndex));
				baseVertexLabelVector.push_back(labelVertexIterator->first);
			}
		}

		outputQuery(baseVertexIdVector, baseVertexLabelVector);

		/*
		* Contain Overlap
		*/
		for (int containQueryIndex = 0; containQueryIndex < QUERY_GROUP_SIZE; containQueryIndex++) {
			std::vector<int> containVertexLabelVector(baseVertexLabelVector);
			std::vector<int> containVertexIdVector(baseVertexIdVector);

			std::set<int> containVertexIdSet(baseVertexIdSet);
			std::set<int> containVertexLabelSet(baseVertexIdSet);


			std::uniform_int_distribution<int> distribution_query_size(BASE_QUERY_SIZE + 1, MAXIMUM_QUERY_SIZE);
			size_t querySize = distribution_query_size(generator);


			while (containVertexIdVector.size() < querySize) {

				std::uniform_int_distribution<int> distribution_vertex_next_id(0, containVertexIdVector.size() - 1);
				attachVertex = dataGraph->getVertexAddressByVertexId(containVertexIdVector[distribution_vertex_next_id(generator)]);
				std::uniform_int_distribution<int> distribution_vertex_label(0, attachVertex->labelVertexList.size() - 1);

				std::map<int, vector<int>>::iterator labelVertexIterator = attachVertex->labelVertexList.begin();
				std::advance(labelVertexIterator, distribution_vertex_label(generator));

				std::uniform_int_distribution<int> distribution_vertex_id(0, labelVertexIterator->second.size() - 1);
				int neighbourIndex = distribution_vertex_id(generator);
				if (containVertexIdSet.insert(labelVertexIterator->second.at(neighbourIndex)).second == true) {

					containVertexIdVector.push_back(labelVertexIterator->second.at(neighbourIndex));
					containVertexLabelVector.push_back(labelVertexIterator->first);
				}
			}

			outputQuery(containVertexIdVector, containVertexLabelVector);

			/*
			* duplicate Overlap
			*/
			if (containQueryIndex == 2) {
				outputQuery(containVertexIdVector, containVertexLabelVector);
			}
		}
	}
}


void CoreSubgraphSpanOverlapQG::generatePaths(std::vector<int> & coreVerticesVector) {
	//TODO
}




void CoreSubgraphSpanOverlapQG::outputQuery(std::vector<int> & vertexIdVector, std::vector<int> & vertexLabelVector) {
	cout << "A new query " << queryGraphIndex << endl;
	(*resultFile) << "t # " << queryGraphIndex << endl;


	for (size_t i = 0; i < vertexLabelVector.size(); i++) {
		(*resultFile) << "v " << i << " " << vertexLabelVector[i] << endl;
	}

	for (size_t i = 0; i < vertexIdVector.size(); i++) {
		for (size_t j = i + 1; j < vertexIdVector.size(); j++) {
			if (dataGraph->edge(vertexIdVector[i], vertexIdVector[j])) {
				(*resultFile) << "e " << i << " " << j << " 0" << endl;
			}
			else if (i > 9) {
				std::uniform_int_distribution<int> distribution_query_confuse(0, 2);
				if (distribution_query_confuse(generator) == 0) {
					(*resultFile) << "e " << i << " " << j << " 0" << endl;
				}
			}
			else {
				if (i == 2 && j % 2 == 0) {
					(*resultFile) << "e " << i << " " << j << " 0" << endl;
				}
			}
		}
	}

	queryGraphIndex++;
}

