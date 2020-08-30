#include "SimiMethodTest.h"
#include "TimeUtility.h"
#include "MatrixUtility.h"
#include "FindMaximumCommonSubgraph.h"
#include "FindAllMaximalCommonSubgraphs.h"
#include<algorithm>


using namespace std;


SimiMethodTest::SimiMethodTest(std::vector<AdjacenceListsGRAPH>* pQueryGraphVector, std::ofstream * pResultFile)
{
	queryGraphVector = pQueryGraphVector;

	resultFile = pResultFile;
	/*
	* use a matrix to save the tls information between queries
	*/
	tlsGraphMatrix = new float*[queryGraphVector->size()];
	for (unsigned int i = 0; i < queryGraphVector->size(); i++) {
		tlsGraphMatrix[i] = new float[queryGraphVector->size()];
		for (unsigned int j = 0; j < queryGraphVector->size(); j++) {
			tlsGraphMatrix[i][j] = 0;
		}
		tlsGraphMatrix[i][i] = 0;
	}

	jaccardGraphMatrix = new float*[queryGraphVector->size()];
	for (unsigned int i = 0; i < queryGraphVector->size(); i++) {
		jaccardGraphMatrix[i] = new float[queryGraphVector->size()];
		for (unsigned int j = 0; j < queryGraphVector->size(); j++) {
			jaccardGraphMatrix[i][j] = 0;
		}
		jaccardGraphMatrix[i][i] = 0;
	}

	mcsStoreMatrix = new int*[queryGraphVector->size()];
	for (unsigned int i = 0; i < queryGraphVector->size(); i++) {
		mcsStoreMatrix[i] = new int[queryGraphVector->size()];
		for (unsigned int j = 0; j < queryGraphVector->size(); j++) {
			mcsStoreMatrix[i][j] = 0;
		}
		mcsStoreMatrix[i][i] = 0;
	}
}

SimiMethodTest::~SimiMethodTest()
{
	/*
	 * clean the member functions used
	 */
	for (unsigned int i = 0; i < queryGraphVector->size(); ++i) {
		delete[] tlsGraphMatrix[i];
	}
	delete[] tlsGraphMatrix;

	for (unsigned int i = 0; i < queryGraphVector->size(); ++i) {
		delete[] jaccardGraphMatrix[i];
	}
	delete[] jaccardGraphMatrix;

	for (unsigned int i = 0; i < queryGraphVector->size(); ++i) {
		delete[] mcsStoreMatrix[i];
	}
	delete[] mcsStoreMatrix;
}


void SimiMethodTest::execute()
{
	/*
	 * compute the tls matrix based on the tls similarity
	 */
	TLSGraph * tlsGraph1 = new TLSGraph(tlsGraphMatrix, queryGraphVector, resultFile);
	tlsGraph1->buildTLSGraph(0);
	delete tlsGraph1;

#ifdef _DEBUG
	(*resultFile) << endl << "TLS Simi Matrix " << endl;
	MatrixUtility::outputFloatMatrix(tlsGraphMatrix, (*queryGraphVector).size(), (*queryGraphVector).size(), resultFile);
#endif

	/*
	 * compute the tls matrix based on the jaccard similarity
	 */
	TLSGraph * tlsGraph2 = new TLSGraph(jaccardGraphMatrix, queryGraphVector, resultFile);
	tlsGraph2->buildTLSGraph(1);
	delete tlsGraph2;

#ifdef _DEBUG
	(*resultFile) << endl << "Jaccard Simi Matrix " << endl;
	MatrixUtility::outputFloatMatrix(jaccardGraphMatrix, (*queryGraphVector).size(), (*queryGraphVector).size(), resultFile);
#endif

	/*
	* We use a matrix to save the number of MCSs for each query-pair.
	*/
	int totalNumberOfMCS = 0;
	for (size_t i = 0; i < (*queryGraphVector).size(); i++) {
		AdjacenceListsGRAPH * iQuery = &(*queryGraphVector)[i];
		for (size_t j = i + 1; j < (*queryGraphVector).size(); j++) {
			AdjacenceListsGRAPH * jQuery = &(*queryGraphVector)[j];


			if (jaccardGraphMatrix[i][j] == 0) {
				continue;
			}
			if (tlsGraphMatrix[i][j] == 1) {
				totalNumberOfMCS++;
				if (iQuery->getNumberOfEdges() > jQuery->getNumberOfEdges()) {
					mcsStoreMatrix[i][j] = jQuery->getNumberOfEdges();
				}
				else {
					mcsStoreMatrix[i][j] = iQuery->getNumberOfEdges();
				}
				continue;
			}

			cout << "Compute MCS To " << i << " " << j <<endl;
			
			
			int mcsNo = FindAllMaximalCommonSubgraphs::computeAllMaximalCommonSubgraphs(iQuery, jQuery, NULL);
			mcsStoreMatrix[i][j] = mcsNo;
			totalNumberOfMCS += mcsNo;

			/*AdjacenceListsGRAPH * mcsGraph = new AdjacenceListsGRAPH();
			FindMaximumCommonSubgraph::computeMaximumCommonSubgraph(iQuery, jQuery, mcsGraph);

			if (mcsGraph->getNumberOfEdges() >= 2) {
				mcsStoreMatrix[i][j] = mcsGraph->getNumberOfEdges();
				totalNumberOfMCS++;

				cout << " MCS number: " << mcsGraph->getNumberOfEdges()  << endl;
			}
			else {
				cout << " Empty " << endl;
			}
			delete mcsGraph; */
		}
	}

#ifdef _DEBUG
	(*resultFile) << endl << "MCS Store Matrix " << endl;
	MatrixUtility::outputIntMatrix(mcsStoreMatrix, (*queryGraphVector).size(), (*queryGraphVector).size(), resultFile); //@debug
#endif


	/*
	* we store both the similarity to a list as well
	*/
	for (size_t i = 0; i < (*queryGraphVector).size(); i++) {
		for (size_t j = i + 1; j < (*queryGraphVector).size(); j++) {
			tlsSimiList.push_back(std::pair<float, int>(tlsGraphMatrix[i][j], i*(*queryGraphVector).size() + j));
			jaccardSimiList.push_back(std::pair<float, int>(jaccardGraphMatrix[i][j], i*(*queryGraphVector).size() + j));
		}
	}
	std::sort(tlsSimiList.begin(), tlsSimiList.end(), simi_greater());
	std::sort(jaccardSimiList.begin(), jaccardSimiList.end(), simi_greater());


	int tlsNumberOfZero = 0;
	for (int i = 0; i < tlsSimiList.size(); i++) {
		if (tlsSimiList[i].first == 0) {
			tlsNumberOfZero++;
		}
	}

	int jaccardNumberOfZero = 0;
	for (int i = 0; i < jaccardSimiList.size(); i++) {
		if (jaccardSimiList[i].first == 0) {
			jaccardNumberOfZero++;
		}
	}

	/*
	 * Given the minimum required percent of the MCS, get the number of computations tls needed to get this number of MCS
	 */
	for (float miniPercent = 0.1; miniPercent < 1; miniPercent += 0.1) {
		int miniNumberMCSRequired = totalNumberOfMCS * miniPercent;

		int tlsComputations = 0;
		int tlsMCSFounded = 0;
		while (tlsMCSFounded < miniNumberMCSRequired && tlsComputations < tlsSimiList.size()) {
			
			int i = tlsSimiList[tlsComputations].second / (*queryGraphVector).size();
			int j = tlsSimiList[tlsComputations].second % (*queryGraphVector).size();

			if (mcsStoreMatrix[i][j] > 0) {
				tlsMCSFounded++;
			}

			tlsComputations++;
		}

		

		int jaccardComputations = 0;
		int jaccardMCSFounded = 0;
		while (jaccardMCSFounded < miniNumberMCSRequired && jaccardComputations < jaccardSimiList.size()) {

			int i = jaccardSimiList[jaccardComputations].second / (*queryGraphVector).size();
			int j = jaccardSimiList[jaccardComputations].second % (*queryGraphVector).size();

			if (mcsStoreMatrix[i][j] > 0) {
				jaccardMCSFounded++;
			}

			jaccardComputations++;
		}

		(*resultFile) << "Required Percent : " << miniPercent<< ":" << miniNumberMCSRequired << " TLS Compuations: " << tlsComputations << endl;
		(*resultFile) << "Required Percent : " << miniPercent << ":" << miniNumberMCSRequired << " Jaccard Compuations: " << jaccardComputations << endl;

	}

}
