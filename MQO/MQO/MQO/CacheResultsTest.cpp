#include "CacheResultsTest.h"
#include"CacheEmbeddingNode.h"
#include "TimeUtility.h"

CacheResultsTest::CacheResultsTest(AdjacenceListsGRAPH * pDataGraph, std::vector<AdjacenceListsGRAPH>* pQueryGraphVector, std::ofstream * pResultFile)
{
	dataGraph = pDataGraph;
	queryGraphVector = pQueryGraphVector;
	resultFile = pResultFile;


	linkedListsMSize = new long long[5];
	linkedListRetriTime = new double[5];

	batchResultCaches = new std::vector<std::vector<CacheEmbeddingNode *>>[5];

	for (int parWidthIndex = 0; parWidthIndex < 5; parWidthIndex++) {

		linkedListsMSize[parWidthIndex] = 0;
		linkedListRetriTime[parWidthIndex] = 0;

		for (int queryGraphId = 0; queryGraphId < queryGraphVector->size(); queryGraphId++) {
			batchResultCaches[parWidthIndex].push_back(std::vector<CacheEmbeddingNode *>());
		}
	}

	for (int queryGraphId = 0; queryGraphId < queryGraphVector->size(); queryGraphId++) {

		numberOfEmbeddings.push_back(0);
		trivialEmbeddingLists.push_back(std::vector<std::vector<int>>());
	}

	trivialEmbeddingListMSize = 0;

	turboIso_CacheResults = new TurboIso_CacheResults(dataGraph, &numberOfEmbeddings, &trivialEmbeddingLists);

}

void CacheResultsTest::execute()
{

	for (int queryGraphId = 0; queryGraphId < queryGraphVector->size(); queryGraphId++) {

		turboIso_CacheResults->setParameters(&(*queryGraphVector)[queryGraphId]);
		turboIso_CacheResults->execute();

		if (trivialEmbeddingLists[queryGraphId].size() == 0) {
			cout << "** Finished One query " << queryGraphId << " Embedding size: " << trivialEmbeddingLists[queryGraphId].size() << endl;
			continue;
		}

		trivialEmbeddingListMSize += trivialEmbeddingLists[queryGraphId].size() * (*queryGraphVector)[queryGraphId].getNumberOfVertexes() * sizeof(int);

		for (int parWidthIndex = 0; parWidthIndex < 5; parWidthIndex++) {
			saveCLLResults(parWidthIndex + 1, queryGraphId, &batchResultCaches[parWidthIndex]);
			computeCLLRetrivingPerfor(queryGraphId, &batchResultCaches[parWidthIndex], &linkedListRetriTime[parWidthIndex]);
			/*
			* Memory Comparison
			*/
			computeMemory(queryGraphId, parWidthIndex);
		}




		trivialEmbeddingLists[queryGraphId] = std::vector<std::vector<int>>();
		for (int parWidthIndex = 0; parWidthIndex < 5; parWidthIndex++) {
			batchResultCaches[parWidthIndex][queryGraphId] = vector<CacheEmbeddingNode*>();
		}

	}

	(*resultFile) << "**** Cached Results Memory Size: " << endl;
	(*resultFile) << "Trivial Embedding List: " << trivialEmbeddingListMSize << " bytes" << endl;

	for (int parWidthIndex = 0; parWidthIndex < 5; parWidthIndex++) {
		(*resultFile) << parWidthIndex << "-- Width--Trivial Compressed Graph: " << linkedListsMSize[parWidthIndex] << " bytes" << endl;
	}


	(*resultFile) << "****Retriving Time: " << endl;
	for (int parWidthIndex = 0; parWidthIndex < 5; parWidthIndex++) {
		(*resultFile) << parWidthIndex << "-- Width--Retriving Time: " << linkedListRetriTime[parWidthIndex] << " milliseconds" << endl;
	}
}


void CacheResultsTest::computeMemory(int queryGraphId, int parWidthIndex)
{
	for (int j = 0; j < batchResultCaches[parWidthIndex][queryGraphId].size(); j++) {
		CacheEmbeddingNode * cllNode = batchResultCaches[parWidthIndex][queryGraphId][j];
		int comboVerticeNumber = 0;
		for (int i = 0; i < 5; i++) {
			if (cllNode->comboMappingVertices[i] != -1) {
				comboVerticeNumber++;
			}
			else {
				break;
			}
		}

		while (cllNode != NULL) {
			linkedListsMSize[parWidthIndex] += comboVerticeNumber * sizeof(int);
			cllNode = cllNode->adj;
		}
	}
}

void CacheResultsTest::saveCLLResults(int parWidth, int queryGraphId, std::vector<std::vector<CacheEmbeddingNode *>> * cachedLinkedLists) {
	int * embeddingArray = new int[(*queryGraphVector)[queryGraphId].getNumberOfVertexes()];
	(*queryGraphVector)[queryGraphId].cleanComboInfo();
	(*queryGraphVector)[queryGraphId].buildComboGraph(parWidth);

	for (unsigned int comboVertexIndex = 0; comboVertexIndex < (*queryGraphVector)[queryGraphId].getComboGraph()->size(); comboVertexIndex++) {
		(*cachedLinkedLists)[queryGraphId].push_back(NULL);
	}

	for (int embeddingIndex = 0; embeddingIndex < trivialEmbeddingLists[queryGraphId].size(); embeddingIndex++) {
		for (int embeddingDataVertexIndex = 0; embeddingDataVertexIndex < (*queryGraphVector)[queryGraphId].getNumberOfVertexes(); embeddingDataVertexIndex++) {
			embeddingArray[embeddingDataVertexIndex] = trivialEmbeddingLists[queryGraphId][embeddingIndex][embeddingDataVertexIndex];
		}
		ComboLinkedLists::addEmbeddingToCache(cachedLinkedLists, &(*queryGraphVector)[queryGraphId], embeddingArray);
	}
	delete[] embeddingArray;
}

void CacheResultsTest::computeCLLRetrivingPerfor(int queryGraphId, std::vector<std::vector<CacheEmbeddingNode *>> * cachedLinkedLists, double *  linkedListRetriTime) {

	AdjacenceListsGRAPH * queryGraph = &(*queryGraphVector)[queryGraphId];

	comboVertexMapping = new CacheEmbeddingNode*[queryGraph->getComboGraph()->size()];
	for (int i = 0; i < queryGraph->getComboGraph()->size(); i++) {
		comboVertexMapping[i] = NULL;
	}

	TimeUtility llTime;
	llTime.StartCounterMill();
	CacheEmbeddingNode * comboVertexStarting = (*cachedLinkedLists)[queryGraphId][queryGraph->getDFSComboVertexOrder()->at(0)];
	while (comboVertexStarting != NULL) {
		comboVertexMapping[queryGraph->getDFSComboVertexOrder()->at(0)] = comboVertexStarting;
		recursiveEnumLinkedLists(1, queryGraph);
		comboVertexStarting = comboVertexStarting->adj;
	}
	(*linkedListRetriTime) += llTime.GetCounterMill();

	delete[] comboVertexMapping;
}


void CacheResultsTest::recursiveEnumLinkedLists(int comboVertexOrder, AdjacenceListsGRAPH * queryGraph) {

	if (comboVertexOrder == queryGraph->getDFSComboVertexOrder()->size()) {
		// TODO anything ??
		return;
	}

	int comboVertexId = queryGraph->getDFSComboVertexOrder()->at(comboVertexOrder);
	ComboNODE & comboVertex = queryGraph->getComboGraph()->at(comboVertexId);

	std::map<int, std::set<CacheEmbeddingNode *>>::iterator comboMappingIterator = comboVertexMapping[comboVertex.parent]->neighboursList.find(comboVertexId);
	for (std::set<CacheEmbeddingNode *>::iterator comboMapping = comboMappingIterator->second.begin(); comboMapping != comboMappingIterator->second.end(); comboMapping++) {

		for (std::set<int>::iterator comboVertexNeiIterator = comboVertex.neighbours.begin(); comboVertexNeiIterator != comboVertex.neighbours.end(); comboVertexNeiIterator++) {
			if (*comboVertexNeiIterator != comboVertex.parent) {
				if (comboVertexMapping[*comboVertexNeiIterator] != NULL) {
					if (*comboVertexNeiIterator < comboVertexId) {
						std::map<int, std::set<CacheEmbeddingNode *>>::iterator nonTreeMappingIter = comboVertexMapping[*comboVertexNeiIterator]->neighboursList.find(comboVertexId);
						if (nonTreeMappingIter->second.find(*comboMapping) == nonTreeMappingIter->second.end()) {
							continue;
						}
					}
					else {
						std::map<int, std::set<CacheEmbeddingNode *>>::iterator nonTreeMappingIter = (*comboMapping)->neighboursList.find(*comboVertexNeiIterator);
						if (nonTreeMappingIter->second.find(comboVertexMapping[*comboVertexNeiIterator]) == nonTreeMappingIter->second.end()) {
							continue;
						}
					}
				}
			}
		}

		comboVertexMapping[comboVertexId] = *comboMapping;
		recursiveEnumLinkedLists(comboVertexOrder + 1, queryGraph);
		comboVertexMapping[comboVertexId] = NULL;
	}

}