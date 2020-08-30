#include "OverlapQueryGenerator.h"
#include"AdjacenceListsGraph.h"
#include<random>
#include<time.h>


/*
* We first find a base subgraph and then find a set of larger graphs that contains this base graph
* We can specify how many graphs for each core-group. This version do the best to spread the label distribution. It requires the data graph
* contains a large number of label
*/

using namespace std;

OverlapQueryGenerator::OverlapQueryGenerator() {}

OverlapQueryGenerator::OverlapQueryGenerator(AdjacenceListsGRAPH * pDataGraph, int pQueryType, std::ofstream * pResultFile, int pNumberOfCores)
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
	QUERY_GROUP_SIZE = 9;

}

OverlapQueryGenerator::~OverlapQueryGenerator()
{}

void OverlapQueryGenerator::generateQueries()
{

	vector<int> largeLabelDataVertices;
	for (vector<AdjacenceListsGRAPH::Vertex>::iterator dataVertexIterator = dataGraph->getVertexList()->begin(); dataVertexIterator != dataGraph->getVertexList()->end(); dataVertexIterator++) {
		if (dataVertexIterator->labelVertexList.size() >= 5) {
			largeLabelDataVertices.push_back(dataVertexIterator->id);
		}
	}

	set<int> coreVerticesSet;
	vector<int> coreVerticesVector;
	std::uniform_int_distribution<int> distribution_vertex_size(0, largeLabelDataVertices.size() - 1);

	for (size_t i = 0; i < largeLabelDataVertices.size(); i++) {
		if (coreVerticesSet.size() < NUMBER_CORES) {
			if (dataGraph->getVertexAddressByVertexId(largeLabelDataVertices[i])->labelVertexList.size() >= 5) {
				if (coreVerticesSet.insert(largeLabelDataVertices[i]).second == true) {
					coreVerticesVector.push_back(largeLabelDataVertices[i]);
				}
			}
		}
		else {
			break;
		}
	}


	if (queryType == 0) {
		generateSubgraphs(coreVerticesVector);
	}
	else {
		generatePaths(coreVerticesVector);
	}

}

void OverlapQueryGenerator::generateSubgraphs(std::vector<int> & coreVerticesVector) {

	
	for (int coreIndex = 0; coreIndex < coreVerticesVector.size(); coreIndex++) {

		std::vector<int> baseVertexLabelVector;
		std::vector<int> baseVertexIdVector;

		std::set<int> baseVertexIdSet;
		std::set<int> baseVertexLabelSet;


		AdjacenceListsGRAPH::Vertex * attachVertex = dataGraph->getVertexAddressByVertexId(coreVerticesVector[coreIndex]);
		
		baseVertexIdSet.insert(coreVerticesVector[coreIndex]);
		baseVertexLabelSet.insert(attachVertex->label);
		baseVertexIdVector.push_back(coreVerticesVector[coreIndex]);
		baseVertexLabelVector.push_back(attachVertex->label);

		
		while (baseVertexIdVector.size() < BASE_QUERY_SIZE) {
			
			std::uniform_int_distribution<int> distribution_vertex_next_id(0, baseVertexIdVector.size() - 1);
			attachVertex = dataGraph->getVertexAddressByVertexId(baseVertexIdVector[distribution_vertex_next_id(generator)]);
			std::uniform_int_distribution<int> distribution_vertex_label(0, attachVertex->labelVertexList.size() - 1);
		
			std::map<int, vector<int>>::iterator labelVertexIterator = attachVertex->labelVertexList.begin();
			std::advance(labelVertexIterator, distribution_vertex_label(generator));
			if (baseVertexLabelSet.find(labelVertexIterator->first) == baseVertexLabelSet.end()) {
				std::uniform_int_distribution<int> distribution_vertex_id(0, labelVertexIterator->second.size() - 1);
				int neighbourIndex = distribution_vertex_id(generator);
				if (baseVertexIdSet.insert(labelVertexIterator->second.at(neighbourIndex)).second == true) {
				
					baseVertexLabelSet.insert(labelVertexIterator->first);
					baseVertexIdVector.push_back(labelVertexIterator->second.at(neighbourIndex));
					baseVertexLabelVector.push_back(labelVertexIterator->first);
				}
			}
		}

		outputQuery(baseVertexIdVector, baseVertexLabelVector);

		/*
		 * Contain Overlap 
		 */
		for (int containQueryIndex = 0; containQueryIndex < QUERY_GROUP_SIZE; containQueryIndex ++) {
			std::vector<int> containVertexLabelVector(baseVertexLabelVector);
			std::vector<int> containVertexIdVector(baseVertexIdVector);

			std::set<int> containVertexIdSet(baseVertexIdSet);
			std::set<int> containVertexLabelSet(baseVertexIdSet);


			std::uniform_int_distribution<int> distribution_query_size(BASE_QUERY_SIZE + 1, MAXIMUM_QUERY_SIZE);
			int querySize = distribution_query_size(generator);

			while (containVertexIdVector.size() < querySize) {

				std::uniform_int_distribution<int> distribution_vertex_next_id(0, baseVertexIdVector.size() - 1);
				attachVertex = dataGraph->getVertexAddressByVertexId(baseVertexIdVector[distribution_vertex_next_id(generator)]);
				std::uniform_int_distribution<int> distribution_vertex_label(0, attachVertex->labelVertexList.size() - 1);

				std::map<int, vector<int>>::iterator labelVertexIterator = attachVertex->labelVertexList.begin();
				std::advance(labelVertexIterator, distribution_vertex_label(generator));
				if (containVertexLabelSet.find(labelVertexIterator->first) == containVertexLabelSet.end()) {
					std::uniform_int_distribution<int> distribution_vertex_id(0, labelVertexIterator->second.size() - 1);
					int neighbourIndex = distribution_vertex_id(generator);
					if (containVertexIdSet.insert(labelVertexIterator->second.at(neighbourIndex)).second == true) {

						containVertexLabelSet.insert(labelVertexIterator->first);
						containVertexIdVector.push_back(labelVertexIterator->second.at(neighbourIndex));
						containVertexLabelVector.push_back(labelVertexIterator->first);
					}
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


void OverlapQueryGenerator::generatePaths(std::vector<int> & coreVerticesVector) {
	//TODO
}




void OverlapQueryGenerator::outputQuery(std::vector<int> & vertexIdVector, std::vector<int> & vertexLabelVector) {
	cout << "A new query " << queryGraphIndex << endl;
	(*resultFile) << "t # " << queryGraphIndex++ << endl;


	for (size_t i = 0; i < vertexLabelVector.size(); i++) {
		/*if (i % 4 == 0) {
			// confuse
			(*resultFile) << "v " << i << " " << vertexLabelVector[i] + 1<< endl;
		}
		else {
			(*resultFile) << "v " << i << " " << vertexLabelVector[i] << endl;
		}*/

		(*resultFile) << "v " << i << " " << vertexLabelVector[i] << endl;
	}

	for (size_t i = 0; i < vertexIdVector.size(); i++) {
		for (size_t j = i + 1; j < vertexIdVector.size(); j++) {
			if (dataGraph->edge(vertexIdVector[i], vertexIdVector[j])) {
				(*resultFile) << "e " << i << " " << j << " 0" << endl;
			}
			else if (i > 9) {
				std::uniform_int_distribution<int> distribution_query_confuse(0, 3);
				if (distribution_query_confuse(generator) == 0) {
					(*resultFile) << "e " << i << " " << j << " 0" << endl;
				}
			}
			else {
				if (i == 4 && j % 4 == 0) {
					(*resultFile) << "e " << i << " " << j << " 0" << endl;
				}
			}
		}
	}
}

