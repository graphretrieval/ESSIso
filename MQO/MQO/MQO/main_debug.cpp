#include<iostream>
#include<fstream>
#include"PCMBuilder.h"
#include"TimeUtility.h"
#include"MQO.h"
#include"SQP.h"
#include"InputCommandLineParser.h"
#include"QueryGraphIsomorphismTest.h"
#include"GlobalConstant.h"
#include"FrequentPatternGroup.h"
#include"DebugGraphOverlapQG.h"


using namespace std;
 

static std::vector<AdjacenceListsGRAPH> dataGraphVector, queryGraphVector;


static void loadDataGraphs(string dataGraphFileName);
static void loadQueryGraphs(string queryGraphFileName);



/**
 * This main is used for Debug perpurse. It is handy to use _DEBUG to limit to hide/show some code
 * while sometimes a seperate debug_main will help to test something very seperately
 * You need to rename the main_debug to main in order to let the system pick this main 
 * also you need to rename the main function in the main file as something else temporarily
 */
int main_debug(int argc, char* argv[]) {

	GlobalConstant::G_RUNNING_OPTION_INDEX = GlobalConstant::RUN_OP_TURBOISO;
	GlobalConstant::G_GOURP_QUERY_CLIQUE_MINI_SIZE = 2;
	GlobalConstant::G_MAXIMUM_WIDTH_COMBO = 3;


	/*std::ofstream resultFile = std::ofstream("C:/Users/s2813995/Dropbox/HectorResearch/PaperProjects/MQP_Center/MQOTestData/human_test.igraph");

	//std::ofstream resultFile2 = std::ofstream("C:/Users/s2813995/Dropbox/HectorResearch/PaperProjects/MQP_Center/MQOTestData/human_test2.igraph");

	loadDataGraphs("C:/Users/s2813995/Dropbox/HectorResearch/PaperProjects/MQP_Center/MQOTestData/human.graph");

	DebugGraphOverlapQG queryGenerator(&dataGraphVector[0], &resultFile);
	queryGenerator.generateQueries(); */
	

	std::ofstream resultFile = std::ofstream("C:/Users/s2813995/Dropbox/HectorResearch/PaperProjects/MQP_Center/MQOTestData/human_test.result");

	loadDataGraphs("C:/Users/s2813995/Dropbox/HectorResearch/PaperProjects/MQP_Center/MQOTestData/human.graph");
	loadQueryGraphs("C:/Users/s2813995/Dropbox/HectorResearch/PaperProjects/MQP_Center/MQOTestData/human_test3.igraph");



	MQO mqo(&dataGraphVector, &queryGraphVector, &resultFile);
	TimeUtility tMQO;
	tMQO.StartCounterMill();
	mqo.buildPCM();
	mqo.orederedQueryProcessing();
	resultFile << "2. MQO Time: " << tMQO.GetCounterMill() << "(milliseconds)" << endl;



	SQP sqo(&dataGraphVector, &queryGraphVector, &resultFile);
	TimeUtility tSQP;
	tSQP.StartCounterMill();
	sqo.queryProcessing();
	resultFile << endl << "1. SQO Average Time: " << tSQP.GetCounterMill() << "(milliseconds)" << endl;

	resultFile.close();

	system("pause");

	return 0;
}

void loadDataGraphs(string dataGraphFileName) {
	std::ifstream dataGraphFile = std::ifstream(dataGraphFileName);
	if (!dataGraphFile) {
		cout << "The data file '" << dataGraphFileName << "' doesn't exist." << endl;
	}
	AdjacenceListsGRAPH_IO::loadGraphFromFile(dataGraphFile, dataGraphVector, true);

	for (vector<AdjacenceListsGRAPH>::iterator dataGraphIterator = dataGraphVector.begin(); dataGraphIterator != dataGraphVector.end(); dataGraphIterator++) {
		dataGraphIterator->buildLabelVertexList();
		dataGraphIterator->buildVertexLabelVertexList();
	}

	cout << "0. Load data graphs and build data graph index done. " << dataGraphFileName << endl;
}

void loadQueryGraphs(string queryGraphFileName) {

	std::ifstream queryGraphFile = std::ifstream(queryGraphFileName);
	if (!queryGraphFile) {
		cout << "The query file '" << queryGraphFileName << "' doesn't exist." << endl;
	}
	AdjacenceListsGRAPH_IO::loadGraphFromFile(queryGraphFile, queryGraphVector, false);


	int queryGraphIndex = 0;
	for (vector<AdjacenceListsGRAPH>::iterator queryGraphIterator = queryGraphVector.begin(); queryGraphIterator != queryGraphVector.end(); queryGraphIterator++) {

		queryGraphIterator->graphId = queryGraphIndex++;
		/*
		* build inverted indexes for each query graph
		*/
		queryGraphIterator->buildDFSTraversalOrder();
		queryGraphIterator->buildVertexLabelVertexList();
		queryGraphIterator->buildLabelVertexList();

		queryGraphIterator->buildVertexLabelEdgeList(); // used by mcs computation
		queryGraphIterator->buildTLSequence(); // used by pcm computation

		queryGraphIterator->buildComboGraph(GlobalConstant::G_MAXIMUM_WIDTH_COMBO);

	}

	cout << "1. Load query graphs and build query graph index done. " << queryGraphFileName << endl;
}

