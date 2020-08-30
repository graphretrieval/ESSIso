#include"Similarity.h"
#include <math.h> 
#include<vector>


using namespace std;

float Similarity::tlsSimilarity(AdjacenceListsGRAPH * graph1, AdjacenceListsGRAPH * graph2)
{
	std::map<TLSequence, std::vector<std::vector<int>>> * tlsSequenceMap1 = graph1->getTLSequenceMap();
	std::map<TLSequence, std::vector<std::vector<int>>> * tlsSequenceMap2 = graph2->getTLSequenceMap();


	// compute the common tls and the mini size of tls instances between two graphs
	std::map<TLSequence, int> commonTlsSequence;
	for (std::map<TLSequence, std::vector<std::vector<int>>>::iterator tlsIterator = tlsSequenceMap1->begin(); tlsIterator != tlsSequenceMap1->end(); tlsIterator++) {
		std::map<TLSequence, std::vector<std::vector<int>>>::iterator innterTlsIterator = tlsSequenceMap2->find(tlsIterator->first);
		if (innterTlsIterator != tlsSequenceMap2->end()) {
			int miniTlsSize = 0;
			if (tlsIterator->second.size() < innterTlsIterator->second.size()) {
				miniTlsSize = tlsIterator->second.size();
			}
			else {
				miniTlsSize = innterTlsIterator->second.size();
			}
			commonTlsSequence.insert(std::pair<TLSequence, int>(tlsIterator->first, miniTlsSize));
		}
	}

	// empoty common check
	if (commonTlsSequence.size() == 0) {
		return 0;
	}


	/**
	* compute the connected graph size for both graph 1 and 2
	* TODO, here we only select the first "commonTlsIterator->second" instances for each TLS, however, a enumeration should be tried to get the maximum linked graph size
	*/
	vector<vector<int>> coveredTlsInstance1;
	vector<vector<int>> coveredTlsInstance2;
	for (std::map<TLSequence, int>::iterator commonTlsIterator = commonTlsSequence.begin(); commonTlsIterator != commonTlsSequence.end(); commonTlsIterator++) {
		for (int tlsInstanceIndex = 0; tlsInstanceIndex < commonTlsIterator->second; tlsInstanceIndex++) {
			coveredTlsInstance1.push_back(tlsSequenceMap1->find(commonTlsIterator->first)->second[tlsInstanceIndex]);
			coveredTlsInstance2.push_back(tlsSequenceMap2->find(commonTlsIterator->first)->second[tlsInstanceIndex]);
		}
	}
	int maxConnectedSize1 = computeMaxConnectedTlsSize(&coveredTlsInstance1);
	int maxConnectedSize2 = computeMaxConnectedTlsSize(&coveredTlsInstance2);

	if (maxConnectedSize1 < maxConnectedSize2) {
		return (float)maxConnectedSize1 / graph1->getTotalTLSNumber();
	}
	else if(maxConnectedSize1 > maxConnectedSize2){
		return (float)maxConnectedSize2 / graph2->getTotalTLSNumber();
	}
	else {
		// when the max connected instance is the same size. we choose a graph with less tlss(smaller graph)
		if (graph1->getTotalTLSNumber() <= graph2->getTotalTLSNumber()) {
			return (float)maxConnectedSize1 / graph1->getTotalTLSNumber();
		}
		else {
			return (float)maxConnectedSize2 / graph2->getTotalTLSNumber();
		}
	}
}

float Similarity::jaccardSimilarity(AdjacenceListsGRAPH * graph1, AdjacenceListsGRAPH * graph2)
{
	std::map<std::pair<int, int>, std::vector<std::pair<int, int>>> * edgeIndex1 = graph1->getVertexLabelsEdgeList();
	std::map<std::pair<int, int>, std::vector<std::pair<int, int>>> * edgeIndex2 = graph2->getVertexLabelsEdgeList();

	int numberOfJoin = 0;
	for (std::map<std::pair<int, int>, std::vector<std::pair<int, int>>>::iterator edgeLabelIterator = edgeIndex1->begin(); edgeLabelIterator != edgeIndex1->end(); edgeLabelIterator++) {
		std::map<std::pair<int, int>, std::vector<std::pair<int, int>>>::iterator edgeLabelIterator2 = edgeIndex2->find(edgeLabelIterator->first);
		if (edgeLabelIterator2 != edgeIndex2->end()) {
			numberOfJoin++;
		}
	}

	float result = (float)numberOfJoin / (edgeIndex1->size() + edgeIndex2->size() - numberOfJoin);
	return result;
}

int Similarity::computeMaxConnectedTlsSize(std::vector<std::vector<int>>* coveredTlsInstances)
{

	int maxConnectedTlsSize = 0;
	int maxInstanceEdgeNo = 0;

	bool * flags = new bool[coveredTlsInstances->size()];
	for (size_t tlsIntanceIndex = 0; tlsIntanceIndex < coveredTlsInstances->size(); tlsIntanceIndex++) {
		flags[tlsIntanceIndex] = false;
	}
	int tlsTested = coveredTlsInstances->size();
	while (tlsTested-- > 0) {

		int connectedTlsSize = 0;
		set<int> coveredVertices1;
		set<std::pair<int, int>> coveredEdges1;

		for (size_t tlsIntanceIndex1 = 0; tlsIntanceIndex1 < coveredTlsInstances->size(); tlsIntanceIndex1++) {
			if (flags[tlsIntanceIndex1]) {
				continue;
			}
			coveredVertices1.insert(coveredTlsInstances->at(tlsIntanceIndex1).begin(), coveredTlsInstances->at(tlsIntanceIndex1).end());
			flags[tlsIntanceIndex1] = true;
			connectedTlsSize++;
			tlsTested--;

			for (size_t tlsIntanceIndex2 = 0; tlsIntanceIndex2 < coveredTlsInstances->size(); tlsIntanceIndex2++) {
				if (flags[tlsIntanceIndex2]) {
					continue;
				}
				for (vector<int>::iterator vertexIterator = coveredTlsInstances->at(tlsIntanceIndex2).begin(); vertexIterator != coveredTlsInstances->at(tlsIntanceIndex2).end(); vertexIterator++) {
					if (coveredVertices1.find(*vertexIterator) != coveredVertices1.end()) {
						flags[tlsIntanceIndex2] = true;
						coveredVertices1.insert(coveredTlsInstances->at(tlsIntanceIndex2).begin(), coveredTlsInstances->at(tlsIntanceIndex2).end());

						coveredEdges1.insert(std::pair<int, int>(coveredTlsInstances->at(tlsIntanceIndex2)[0], coveredTlsInstances->at(tlsIntanceIndex2)[1]));
						coveredEdges1.insert(std::pair<int, int>(coveredTlsInstances->at(tlsIntanceIndex2)[1], coveredTlsInstances->at(tlsIntanceIndex2)[2]));

						connectedTlsSize++;
						tlsTested--;
						break;
					}
				}
			}
			
		}


		if (connectedTlsSize > maxConnectedTlsSize) {
			maxConnectedTlsSize = connectedTlsSize;
			maxInstanceEdgeNo = coveredEdges1.size();
		}
	}
	
	delete[] flags;

	//return maxConnectedTlsSize;
	//return maxInstanceEdgeNo;
	int totalConnectedTlsSize = coveredTlsInstances->size();
	return totalConnectedTlsSize;
}
