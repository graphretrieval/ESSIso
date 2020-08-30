#include"TLSGraph.h"
#include"Similarity.h"
#include"AdjacenceListsGRAPH_IO.h"
#include"GlobalConstant.h"
#include<vector>
#include<vector>

using namespace std;


TLSGraph::TLSGraph(float ** pTlsGraphMatrix, std::vector<AdjacenceListsGRAPH> * pQueryGraphVector, std::ofstream * pResultFile) {
	tlsGraphMatrix = pTlsGraphMatrix;
	queryGraphVector = pQueryGraphVector;
	resultFile = pResultFile;
}


void TLSGraph::buildTLSGraph(int similarityIndex) {

	for (size_t  i = 0; i < (*queryGraphVector).size(); i++) {

		AdjacenceListsGRAPH * iQuery = &(*queryGraphVector)[i];

		for (size_t  j = i + 1; j < (*queryGraphVector).size(); j++) {
			AdjacenceListsGRAPH * jQuery = &(*queryGraphVector)[j];
			switch (similarityIndex) {
			case 0:
				tlsGraphMatrix[i][j] = Similarity::tlsSimilarity(iQuery, jQuery);
				break;
			case 1:
				tlsGraphMatrix[i][j] = Similarity::jaccardSimilarity(iQuery, jQuery);
				break;
			}
		}

	}
}
