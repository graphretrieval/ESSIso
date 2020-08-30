#include"CoreVertexSpanOverlapQG.h"
#include"AdjacenceListsGraph.h"
#include<random>
#include<time.h>


/*
 * We first find a set of core vertices and then find subgraphs around this core vertex. 
 * We can specify how many graphs for each core-group
 */
using namespace std;

CoreVertexSpanOverlapQG::CoreVertexSpanOverlapQG() {}

CoreVertexSpanOverlapQG::CoreVertexSpanOverlapQG(AdjacenceListsGRAPH * pDataGraph, int pNumberOfQueries, double pSimilarRatio, int pQueryType, std::ofstream * pResultFile)
{
	dataGraph = pDataGraph;
	numberOfQueries = pNumberOfQueries;
	numberOfCores = numberOfQueries * pSimilarRatio;
	queryType = pQueryType;
	resultFile = pResultFile;

	MINI_NUMBER_QUERY_SIZE = 10;
	MAX_NUMBER_QUERY_SIZE = 15;

	DUPLICATE_DRAW_PROB = 10 * numberOfCores;

	LABEL_NO_VERTEX_NO_RATIO = 0.1;

	generator = default_random_engine(time(NULL));
}

CoreVertexSpanOverlapQG::~CoreVertexSpanOverlapQG()
{}

void CoreVertexSpanOverlapQG::generateQueries()
{
	queryGraphIndex = 0;

	std::uniform_int_distribution<int> distribution_vertex_size(0, dataGraph->getVertexList()->size() - 1);

	set<int> coreVerticesSet;
	vector<int> coreVerticesVector;
	while (coreVerticesSet.size() < numberOfCores) {

		int coreVertexId = distribution_vertex_size(generator);
		if (coreVerticesSet.insert(coreVertexId).second == true) {
			coreVerticesVector.push_back(coreVertexId);
		}
	}

	if (queryType == 0) {
		generateSubgraphs(coreVerticesVector);
	}
	else {
		generatePaths(coreVerticesVector);
	}

}

void CoreVertexSpanOverlapQG::generateSubgraphs(std::vector<int> & coreVerticesVector) {

	std::uniform_int_distribution<int> distribution_query_size(MINI_NUMBER_QUERY_SIZE, MAX_NUMBER_QUERY_SIZE);
	std::uniform_int_distribution<int> distribution_core_vertex(0, numberOfCores - 1);

	while (queryGraphIndex < numberOfQueries) {

		int coreVertexId = coreVerticesVector[distribution_core_vertex(generator)];
		int querySize = distribution_query_size(generator);

		std::vector<std::pair<int, AdjacenceListsGRAPH::Vertex *>> queryVertexArr;
		std::set<int> queryVertexLabelSet;
		set<int> queryVertexIdSet;

		AdjacenceListsGRAPH::Vertex * attachVertex = dataGraph->getVertexAddressByVertexId(coreVertexId);
		queryVertexArr.push_back(std::pair<int, AdjacenceListsGRAPH::Vertex *>(coreVertexId, attachVertex));
		queryVertexIdSet.insert(coreVertexId);
		queryVertexLabelSet.insert(attachVertex->label);

		int triedTimes = 0;
		while (queryVertexIdSet.size() <= querySize) {
			triedTimes++;
			if (triedTimes > 10 * querySize) {
				break;
			}
			/*
			* get a random vertex which has already been added
			*/
			std::uniform_int_distribution<int> distribution_vertex_number(0, queryVertexIdSet.size() - 1);
			int attachVertexIndex = distribution_vertex_number(generator);
			attachVertex = queryVertexArr[attachVertexIndex].second;

			if (attachVertex->inDegree < 1) {
				continue;
			}
			std::uniform_int_distribution<int> distribution_vertex_neighbour(0, attachVertex->inDegree - 1);
			int neighbourIndex = distribution_vertex_neighbour(generator);

			AdjacenceListsGRAPH::link t = attachVertex->adj;
			for (int i = 0; i < neighbourIndex; i++) {
				t = t->next;
			}
			if (queryVertexIdSet.insert(t->v).second == true) {
				queryVertexArr.push_back(std::pair<int, AdjacenceListsGRAPH::Vertex *>(t->v, dataGraph->getVertexAddressByVertexId(t->v)));
				queryVertexLabelSet.insert(dataGraph->getVertexAddressByVertexId(t->v)->label);
			}
		}
		if (triedTimes > 10 * querySize) {
			continue;
		}
		if (queryVertexLabelSet.size() < LABEL_NO_VERTEX_NO_RATIO * querySize) {
			continue;
		}

		/*
		* set the edges
		*/
		vector<std::pair<int, int>> edgeSet;
		for (size_t i = 0; i < queryVertexIdSet.size(); i++) {
			for (size_t j = i + 1; j < queryVertexIdSet.size(); j++) {
				if (dataGraph->edge(queryVertexArr[i].first, queryVertexArr[j].first)) {
					edgeSet.push_back(std::pair<int, int>(i, j));
				} else if (i == 3 && j % 3 == 0) {
					/*
					 * Confusion
					 */
					//edgeSet.push_back(std::pair<int, int>(i, j));
				}
			}
		}

		/*
		* output this query graph
		*/
		outputQuery(queryVertexArr, edgeSet);
		std::uniform_int_distribution<int> distribution_duplicate_prob(0, DUPLICATE_DRAW_PROB);
		int duplicate_draw = distribution_duplicate_prob(generator);
		if (duplicate_draw == 0) {
			// cout << "Bingo, duplicate one" << endl; //debug
			outputQuery(queryVertexArr, edgeSet);
		}
	}
}

void CoreVertexSpanOverlapQG::outputQuery(std::vector<std::pair<int, AdjacenceListsGRAPH::Vertex *>> &  queryVertexArr, std::vector<std::pair<int, int>> & edgeSet) {
	cout << "A new query " << queryGraphIndex << endl;
	(*resultFile) << "t # " << queryGraphIndex++ << endl;

	for (int i = 0; i < queryVertexArr.size(); i++) {
		(*resultFile) << "v " << i << " " << queryVertexArr[i].second->label << endl;
	}

	for (vector<std::pair<int, int>>::iterator edgeIterator = edgeSet.begin(); edgeIterator != edgeSet.end(); edgeIterator++) {
		(*resultFile) << "e " << edgeIterator->first << " " << edgeIterator->second << " 0" << endl;
	}
}


void CoreVertexSpanOverlapQG::generatePaths(std::vector<int> & coreVerticesVector) {

	std::uniform_int_distribution<int> distribution_query_size(MINI_NUMBER_QUERY_SIZE, MAX_NUMBER_QUERY_SIZE);
	std::uniform_int_distribution<int> distribution_core_vertex(0, numberOfCores - 1);

	while (queryGraphIndex < numberOfQueries) {
		// for each group queries
		int coreVertexId = coreVerticesVector[distribution_core_vertex(generator)];
		int querySize = distribution_query_size(generator);
		vector<std::pair<int, AdjacenceListsGRAPH::Vertex *>> queryVertexArr;
		set<int> queryVertexIdSet;
		std::set<int> queryVertexLabelSet;

		AdjacenceListsGRAPH::Vertex * attachVertex = dataGraph->getVertexAddressByVertexId(coreVertexId);
		queryVertexArr.push_back(std::pair<int, AdjacenceListsGRAPH::Vertex *>(coreVertexId, attachVertex));
		queryVertexIdSet.insert(coreVertexId);
		queryVertexLabelSet.insert(attachVertex->label);

		int triedTimes = 0;
		while (queryVertexIdSet.size() <= querySize) {
			triedTimes++;
			if (triedTimes > 10 * querySize) {
				break;
			}
			/*
			* get a random vertex which has already been added
			*/
			attachVertex = queryVertexArr[queryVertexIdSet.size() - 1].second;

			if (attachVertex->inDegree < 1) {
				continue;
			}
			std::uniform_int_distribution<int> distribution_vertex_neighbour(0, attachVertex->inDegree - 1);
			int neighbourIndex = distribution_vertex_neighbour(generator);

			AdjacenceListsGRAPH::link t = attachVertex->adj;
			for (int i = 0; i < neighbourIndex; i++) {
				t = t->next;
			}
			if (queryVertexIdSet.insert(t->v).second == true) {
				queryVertexArr.push_back(std::pair<int, AdjacenceListsGRAPH::Vertex *>(t->v, dataGraph->getVertexAddressByVertexId(t->v)));
				queryVertexLabelSet.insert(dataGraph->getVertexAddressByVertexId(t->v)->label);
			}
		}
		if (triedTimes > 10 * querySize) {
			continue;
		}
		if (queryVertexLabelSet.size() < LABEL_NO_VERTEX_NO_RATIO * querySize) {
			continue;
		}
		/*
		* set the edges
		*/
		vector<std::pair<int, int>> edgeSet;
		for (int i = 0; i < queryVertexIdSet.size() - 1; i++) {
			edgeSet.push_back(std::pair<int, int>(i, i + 1));
		}

		/*
		* output this query graph
		*/
		outputQuery(queryVertexArr, edgeSet);
		std::uniform_int_distribution<int> distribution_duplicate_prob(0, DUPLICATE_DRAW_PROB);
		int duplicate_draw = distribution_duplicate_prob(generator);
		if (duplicate_draw == 0) {
			// cout << "Bingo, duplicate one" << endl; @debug
			outputQuery(queryVertexArr, edgeSet);
		}
	}
}