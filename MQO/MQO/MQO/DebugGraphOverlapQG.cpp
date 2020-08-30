
/*
* Temparary query generator for debugging purpose
*/

#include"DebugGraphOverlapQG.h"
#include<random>
#include<time.h>

using namespace std;

DebugGraphOverlapQG::DebugGraphOverlapQG(AdjacenceListsGRAPH * pDataGraph, std::ofstream * pResultFile, std::ofstream * pResultFile2)
{
	dataGraph = pDataGraph;
	resultFile = pResultFile;
	resultFile2 = pResultFile2;
}

DebugGraphOverlapQG::DebugGraphOverlapQG(AdjacenceListsGRAPH * pDataGraph, std::ofstream * pResultFile)
{
	dataGraph = pDataGraph;
	resultFile = pResultFile;
}





void DebugGraphOverlapQG::generateQueries()
{
	queryGraphIndex = 0;

	std::default_random_engine generator = default_random_engine(time(NULL));
	std::uniform_int_distribution<int> distribution_vertex_size(0, dataGraph->getVertexList()->size() - 1);

	int BASE_QUERY_SIZE = 5;
	int QUERY_GROUP_SIZE = 2;
	int MAXIMUM_QUERY_SIZE = 7;


	for (int generteIteration = 0; generteIteration < 10; generteIteration++) {

		int coreVertexId = distribution_vertex_size(generator);


		std::vector<int> baseVertexLabelVector;
		std::vector<int> baseVertexIdVector;
		std::set<int> baseVertexIdSet;

		baseVertexLabelVector.push_back(0);
		baseVertexLabelVector.push_back(36);
		baseVertexLabelVector.push_back(4);
		baseVertexLabelVector.push_back(1);
		baseVertexLabelVector.push_back(1);

		baseVertexIdVector.push_back(2705);
		baseVertexIdVector.push_back(2680);
		baseVertexIdVector.push_back(2266);
		baseVertexIdVector.push_back(2688);
		baseVertexIdVector.push_back(2698);

		std::set<int>::iterator it = baseVertexIdSet.begin();

		baseVertexIdSet.insert(it, 2705);
		baseVertexIdSet.insert(it, 2680);
		baseVertexIdSet.insert(it, 2266);
		baseVertexIdSet.insert(it, 2688);
		baseVertexIdSet.insert(it, 2698);



		/*AdjacenceListsGRAPH::Vertex * attachVertex = dataGraph->getVertexAddressByVertexId(coreVertexId);

		baseVertexIdSet.insert(coreVertexId);
		baseVertexIdVector.push_back(coreVertexId);
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

		outputQuery(baseVertexIdVector, baseVertexLabelVector); */

		/*
		* Contain Overlap
		*/
		for (int containQueryIndex = 0; containQueryIndex < QUERY_GROUP_SIZE; containQueryIndex++) {

			std::vector<int> containVertexLabelVector(baseVertexLabelVector);
			std::vector<int> containVertexIdVector(baseVertexIdVector);

			std::set<int> containVertexIdSet(baseVertexIdSet);
			std::set<int> containVertexLabelSet(baseVertexIdSet);


			std::uniform_int_distribution<int> distribution_query_size(BASE_QUERY_SIZE + 1, MAXIMUM_QUERY_SIZE);
			int querySize = distribution_query_size(generator);


			while (containVertexIdVector.size() < querySize) {

				std::uniform_int_distribution<int> distribution_vertex_next_id(0, containVertexIdVector.size() - 1);
				AdjacenceListsGRAPH::Vertex * attachVertex = dataGraph->getVertexAddressByVertexId(containVertexIdVector[distribution_vertex_next_id(generator)]);
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
		}
	}


}

void DebugGraphOverlapQG::outputQuery(std::vector<int> & vertexIdVector, std::vector<int> & vertexLabelVector) {
	cout << "A new query " << queryGraphIndex << endl;
	(*resultFile) << "t # " << queryGraphIndex << endl;
	//(*resultFile2) << "t # " << queryGraphIndex << endl;


	for (int i = 0; i < vertexLabelVector.size(); i++) {
		(*resultFile) << "v " << i << " " << vertexLabelVector[i] << endl;
		//(*resultFile2) << "v " << i << " " << vertexLabelVector[i] << " "<< vertexIdVector[i] << endl;
	}

	for (int i = 0; i < vertexIdVector.size(); i++) {
		for (int j = i + 1; j < vertexIdVector.size(); j++) {
			if (dataGraph->edge(vertexIdVector[i], vertexIdVector[j])) {
				(*resultFile) << "e " << i << " " << j << " 0" << endl;
				//(*resultFile2) << "e " << i << " " << j << " 0" << endl;
			}
		}
	}

	queryGraphIndex++;
}
