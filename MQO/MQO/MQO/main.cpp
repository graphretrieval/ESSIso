#include<iostream>
#include<fstream>
#include"PCMBuilder.h"
#include"TimeUtility.h"
#include"MQO.h"
#include"SQP.h"
#include"InputCommandLineParser.h"
#include"AdjacenceListsGRAPH_BOOST.h"
#include"GlobalConstant.h"
#include"CoreSubgraphSpanOverlapQG.h"
#include"CoreVertexSpanOverlapQG.h"
#include"OverlapQueryGenerator.h"
#include"RandomQG.h"
#include"CacheResultsTest.h"
#include"SimiMethodTest.h"
#include "FindMaximumCommonSubgraph.h"
#include "CommonSubGraph.h"
#include <vector>
#include <string>
#include <chrono>



using namespace std;


static string dataGraphFileName, queryGraphFileName, hyperGraphFileName, containmentGraphFileName, resultFilename;
static std::vector<AdjacenceListsGRAPH> dataGraphVector, queryGraphVector;
static std::ofstream resultFile;

static int queryType = 0;
static int numberOfCores = 0;
static int groupSize = 0;

static void loadDataGraphs();
static void loadQueryGraphs();
static void loadHyperGraphs();
static void loadResultFile(std::vector<string> args);
static void setTheConfigurationSettings(std::vector<string> args);
static void help();



int main(int argc, char* argv[]) {

// #ifdef _DEBUG
// 	argc = 9;
// 	char * debugArg[9] = { "-queryProTest",
// 		"-qg", "../Resources/mqo_test_q.graph",
// 		"-dg", "../Resources/mqo_test_d.graph",
// 		"-subIso", "TURBOISO",
// 		"-out", "../Resources/result.txt"};
// 	argv = debugArg;
// #endif

	std::vector<string> args;
	for (int i=0;i< argc;i++) {
		args.push_back(argv[i]);
	}

	if (InputCommandLineParser::cmdOptionExists(args, "-help")) {
		help();
	}

	/*******************************************
	 * Query Graph Generator
	 *******************************************/
	if (InputCommandLineParser::cmdOptionExists(args, "-gen")) {

		dataGraphFileName = InputCommandLineParser::getCmdOption(args, "-dg");
		queryType = std::stoi(InputCommandLineParser::getCmdOption(args, "-t"));
		numberOfCores = std::stoi(InputCommandLineParser::getCmdOption(args, "-nc"));
		groupSize = std::stoi(InputCommandLineParser::getCmdOption(args, "-gs"));
		

		std::ifstream datagraphfile = std::ifstream(dataGraphFileName);
		if (!datagraphfile) {
			cout << "the data file '" << dataGraphFileName << "' doesn't exist." << endl;
		}
		AdjacenceListsGRAPH_IO::loadGraphFromFile(datagraphfile, dataGraphVector, true);

		//OverlapQueryGenerator overlapQueryGenerator(&dataGraphVector[0], queryType, &resultFile, numberOfCores);
		//overlapQueryGenerator.generateQueries();

		//CoreSubgraphSpanOverlapQG coreSubgraphSpanOverlapQG(&dataGraphVector[0], queryType, &resultFile, numberOfCores, groupSize);
		//coreSubgraphSpanOverlapQG.generateQueries();

		//RandomQG randomQG(&dataGraphVector[0], queryType, &resultFile, numberOfCores);
		//randomQG.generateQueries();

		CoreVertexSpanOverlapQG coreVertexSpanOverlapQG(&dataGraphVector[0], numberOfCores * groupSize, 1 / (double)groupSize, 0, &resultFile);
		coreVertexSpanOverlapQG.generateQueries();

		resultFile.close();
		
		// resultFile << "Queries generated." << endl;

#ifdef _DEBUG
// 		system("pause"); //@debug
#endif

		return 0;
	}


	/*******************************************
	*   Cache Testers *
	*******************************************/
	if (InputCommandLineParser::cmdOptionExists(args, "-cacheTest")) {
		
		loadResultFile(args);

		queryGraphFileName = InputCommandLineParser::getCmdOption(args, "-qg");
		loadQueryGraphs();

		dataGraphFileName = InputCommandLineParser::getCmdOption(args, "-dg");
		loadDataGraphs();

		CacheResultsTest cacheResultsTest(&dataGraphVector[0], &queryGraphVector, &resultFile);
		cacheResultsTest.execute();

		return 0;
	}

	/*******************************************
	*   Similarity Method Testers *
	*******************************************/
	if (InputCommandLineParser::cmdOptionExists(args, "-simiTest")) {
		
		loadResultFile(args);

		queryGraphFileName = InputCommandLineParser::getCmdOption(args, "-qg");
		loadQueryGraphs();

		SimiMethodTest simiMethodTest(&queryGraphVector, &resultFile);
		simiMethodTest.execute();

		return 0;
	}

	
	/*******************************************
	*  PCM scalability test *
	*******************************************/
	if (InputCommandLineParser::cmdOptionExists(args, "-pcmScalaTest")) {
		
		loadResultFile(args);

		queryGraphFileName = InputCommandLineParser::getCmdOption(args, "-qg");
		loadQueryGraphs();

		dataGraphFileName = InputCommandLineParser::getCmdOption(args, "-dg");
		loadDataGraphs();
		
		MQO mqo(&dataGraphVector, &queryGraphVector, &resultFile);
		TimeUtility tMQO;

		tMQO.StartCounterMill();
		mqo.buildPCM();
		// resultFile << "PCM Build Time: " << tMQO.GetCounterMill() << "(milliseconds)" << endl;

		exit(0);
	}


	/*******************************************
	*  Query Processing Testers *
	*******************************************/
	if (InputCommandLineParser::cmdOptionExists(args, "-queryProTest")) {
		std::cout << "1\n";
		loadResultFile(args);
		std::cout << "2\n";
		setTheConfigurationSettings(args);
		std::cout << "3\n";
		queryGraphFileName = InputCommandLineParser::getCmdOption(args, "-qg");
		loadQueryGraphs();
		std::cout << "4\n";
		if (InputCommandLineParser::cmdOptionExists(args, "-dg")) {
			dataGraphFileName = InputCommandLineParser::getCmdOption(args, "-dg");
			std::cout << "5\n";
			loadDataGraphs();
		}
		else if (InputCommandLineParser::cmdOptionExists(args, "-hg")) {
			hyperGraphFileName = InputCommandLineParser::getCmdOption(args, "-hg");
			
			if (InputCommandLineParser::cmdOptionExists(args, "-cg")) {
				containmentGraphFileName = InputCommandLineParser::getCmdOption(args, "-cg");
				std::cout << "6\n";
				loadHyperGraphs();
			}
			else {
				// resultFile << "Wrong Parameters! No containment file specified. " << endl;
				cout << "Wrong Parameters! No containment file specified. " << endl;
				exit(-1);
			}
		}
	
		if (InputCommandLineParser::cmdOptionExists(args, "-mqo")) {
			double overHeadTime = 0;
			if (GlobalConstant::batchSize==0) {
				MQO mqo(&dataGraphVector, &queryGraphVector, &resultFile);
				TimeUtility tMQO;
				TimeUtility tMQO2;
				tMQO.StartCounterMill();
				tMQO2.StartCounterMicro();
				cout << endl << "Start Building PCM... " << endl;
				mqo.buildPCM();

				cout << endl <<"Start Computing Execution order... " << endl;
				mqo.buildQueryExecutionOrder();
				overHeadTime = tMQO2.GetCounterMicro();
				cout << endl << "Start Multi-Query Processing... " << endl;
				mqo.orederedQueryProcessing();
				cout << " Overhead Time: " << overHeadTime << endl;
				cout << " MQO Time: " << tMQO.GetCounterMill() << "(milliseconds)" << endl << endl;
			} else {
				double totalTime = 0;
				int size = (queryGraphVector.size() - 1) / GlobalConstant::batchSize + 1;
				std::vector<AdjacenceListsGRAPH> batchQueryGraphVector;
				std::chrono::steady_clock::time_point sStart;
				for (int k = 0; k < size; ++k) {
					// get range for next set of n elements
					auto start_itr = std::next(queryGraphVector.cbegin(), k*GlobalConstant::batchSize);
					auto end_itr = std::next(queryGraphVector.cbegin(), k*GlobalConstant::batchSize + GlobalConstant::batchSize);

					// allocate memory for the sub-vector
					batchQueryGraphVector.resize(GlobalConstant::batchSize);

					// code to handle the last sub-vector as it might
					// contain less elements
					if (k*GlobalConstant::batchSize + GlobalConstant::batchSize > queryGraphVector.size()) {
						end_itr = queryGraphVector.cend();
						batchQueryGraphVector.resize(queryGraphVector.size() - k*GlobalConstant::batchSize);
					}
					cout << "size" << batchQueryGraphVector.size() << endl;
					// copy elements from the input range to the sub-vector
					std::copy(start_itr, end_itr, batchQueryGraphVector.begin());
					int queryGraphIndex = 0;
					for (vector<AdjacenceListsGRAPH>::iterator queryGraphIterator = batchQueryGraphVector.begin(); queryGraphIterator != batchQueryGraphVector.end(); queryGraphIterator++) 
						queryGraphIterator->graphId = queryGraphIndex++;
					MQO mqo(&dataGraphVector, &batchQueryGraphVector, &resultFile);
					TimeUtility tMQO;
					TimeUtility tMQO2;
					tMQO.StartCounterMill();
        			sStart = std::chrono::steady_clock::now();
					tMQO2.StartCounterMicro();
					cout << endl << "Start Building PCM... " << endl;
					mqo.buildPCM();

					cout << endl <<"Start Computing Execution order... " << endl;
					mqo.buildQueryExecutionOrder();
					overHeadTime += tMQO2.GetCounterMicro();
					cout << endl << "Start Multi-Query Processing... " << endl;
					mqo.orederedQueryProcessing();
					// totalTime += tMQO.GetCounterMill();
					totalTime += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - sStart).count()/1000.0;
				}
				cout << " Overhead Time: " << overHeadTime/1000 << endl;
				cout << " MQO Time: " << totalTime << "(milliseconds)" << endl << endl;
			}
		}
		
		if (InputCommandLineParser::cmdOptionExists(args, "-sqp")) {
			cout << endl << "Start Sequential-Query Processing... " << endl;
			SQP sqo(&dataGraphVector, &queryGraphVector, &resultFile);
			TimeUtility tSQP;
			tSQP.StartCounterMill();
			sqo.queryProcessing();
			cout << endl << " SQO Average Time: " << tSQP.GetCounterMill() << "(milliseconds)" << endl;
			cout << endl << " SQO Average Call: " << sqo.totalCall << endl;
		}
		if (InputCommandLineParser::cmdOptionExists(args, "-mcs")) {
			TimeUtility tMCS;
			// AdjacenceListsGRAPH * mcsGraph = new AdjacenceListsGRAPH();
			// tMCS.StartCounterMill();
			// FindMaximumCommonSubgraph::computeMaximumCommonSubgraph(&(queryGraphVector[0]),&(queryGraphVector[0]), mcsGraph);
			// cout << " MCS Time: " << tMCS.GetCounterMill() << "(milliseconds)" << endl << endl;
			// cout << "Maximum Common Subgraph Size: " << mcsGraph->getVertexList()->size() << endl;
			MCS mcsSolver = MCS();
			for (int i =0 ;i < queryGraphVector.size(); i++) {
				tMCS.StartCounterMill();
				map<int, int> mapNode = mcsSolver.solve(queryGraphVector[0], queryGraphVector[i]);
				cout << " MCS Time: " << tMCS.GetCounterMill() << "(milliseconds)" << endl << endl;
				cout << mapNode.size() << endl;
				for (auto i : mapNode) {
					cout << i.first << ":" << i.second << " ";
				}
				cout << endl;
			}

		} 
	
	}

#ifdef _DEBUG
	// system("pause");
#endif

	return 0;
}


void loadDataGraphs() {

	std::ifstream dataGraphFile = std::ifstream(dataGraphFileName);
	if (!dataGraphFile) {
		// resultFile << "The data file '" << dataGraphFileName << "' doesn't exist." << endl;
		cout << "The data file '" << dataGraphFileName << "' doesn't exist." << endl;
		exit(-1);
	}
	AdjacenceListsGRAPH_IO::loadGraphFromFile(dataGraphFile, dataGraphVector, true);

	for (vector<AdjacenceListsGRAPH>::iterator dataGraphIterator = dataGraphVector.begin(); dataGraphIterator != dataGraphVector.end(); dataGraphIterator++) {
		dataGraphIterator->buildLabelVertexList();
		dataGraphIterator->buildVertexLabelVertexList();
		dataGraphIterator->buildEmbeddingMatrix();
		dataGraphIterator->buildIndex();
	}

	// resultFile << "Load data graphs and build data graph index done. " << dataGraphFileName << endl;
	cout << "Load data graphs and build data graph index done. " << dataGraphFileName << endl;
}

void loadQueryGraphs() {

	std::ifstream queryGraphFile = std::ifstream(queryGraphFileName);
	if (!queryGraphFile) {
		cout << "The query file '" << queryGraphFileName << "' doesn't exist." << endl;
	} else cout << "File exist" << endl;
	AdjacenceListsGRAPH_IO::loadGraphFromFile(queryGraphFile, queryGraphVector, false);


	int queryGraphIndex = 0;
	double buildTime = 0;
	double tlsTime = 0;
	TimeUtility tBuild;
	TimeUtility tlsBuild;
	for (vector<AdjacenceListsGRAPH>::iterator queryGraphIterator = queryGraphVector.begin(); queryGraphIterator != queryGraphVector.end(); queryGraphIterator++) {
		// cout << "queryGraphIndex " << queryGraphIndex << endl;
		queryGraphIterator->graphId = queryGraphIndex++;
		/*
		* build inverted indexes for each query graph
		*/
		tBuild.StartCounterMicro();
		queryGraphIterator->buildDFSTraversalOrder();
		queryGraphIterator->buildVertexLabelVertexList();
		queryGraphIterator->buildLabelVertexList();

		queryGraphIterator->buildVertexLabelEdgeList(); // used by mcs computation
		tlsBuild.StartCounterMicro();
		queryGraphIterator->buildTLSequence(); // used by pcm computation
		tlsTime+=tlsBuild.GetCounterMicro();
		queryGraphIterator->buildComboGraph(GlobalConstant::G_MAXIMUM_WIDTH_COMBO);
		queryGraphIterator->buildEmbeddingMatrix();
		buildTime +=tBuild.GetCounterMicro();
	}
	cout << "Build Time " << buildTime/1000 << endl;
	cout << "Tls Time " << tlsTime/1000 << endl;
	cout << "1. Load query graphs and build query graph index done. " << queryGraphFileName << endl;
}

void loadHyperGraphs() {
	
	std::ifstream hyperGraphFile = std::ifstream(hyperGraphFileName);
	std::ifstream containmentGraphFile = std::ifstream(containmentGraphFileName);

	if (!hyperGraphFile) {
		// resultFile << "The hyperGraphFile '" << hyperGraphFileName << "' doesn't exist." << endl;
		cout << "The hyperGraphFile '" << hyperGraphFileName << "' doesn't exist." << endl;
		exit(-1);
	}
	if (!containmentGraphFile) {
		// resultFile << "The containmentGraphFile '" << containmentGraphFileName << "' doesn't exist." << endl;
		cout << "The containmentGraphFile '" << containmentGraphFileName << "' doesn't exist." << endl;
		exit(-1);
	}

	AdjacenceListsGRAPH_IO::loadGraphFromFile(hyperGraphFile, dataGraphVector, true);
	AdjacenceListsGRAPH_BOOST::loadContainmentGraph(dataGraphVector, containmentGraphFile);

	/*
	* Build indexes for the hypergraph
	* 1. For each label within label set of the graph, we attach all data vertices with this lable to it
	* 2. For each label within the label set of each vertex's neighbours, we attach all its neighbours with this label to it
	* 3. For each label within label set of the graph, we attach the s-roots vertices with this lable to it. s-roots have no s-contained parents
	*/
	for (size_t hyperGraphIndex = 0; hyperGraphIndex < dataGraphVector.size(); hyperGraphIndex++) {
		dataGraphVector[hyperGraphIndex].buildLabelVertexList();
		dataGraphVector[hyperGraphIndex].buildVertexLabelVertexList();
		dataGraphVector[hyperGraphIndex].buildEmbeddingMatrix();
		dataGraphVector[hyperGraphIndex].buildIndex();
		AdjacenceListsGRAPH_BOOST::buildLabelRootMap(dataGraphVector[hyperGraphIndex]);
	}

	// resultFile << " Load hyper graphs and build hyper graph index done. " << hyperGraphFileName << endl;
	cout << " Load hyper graphs and build hyper graph index done. " << hyperGraphFileName << endl;
}

void loadResultFile(std::vector<string> args) {

	if (InputCommandLineParser::cmdOptionExists(args, "-out")) {
		resultFilename = InputCommandLineParser::getCmdOption(args, "-out");
		resultFile = std::ofstream(resultFilename, std::ios_base::app);
	}
	else {
		// resultFile << "Wrong Parameters! No output file specified. Use [-out] " << endl;
		help();
		exit(1);
	}
}

void setTheConfigurationSettings(std::vector<string> args) {

	
	if (InputCommandLineParser::cmdOptionExists(args, "-subIso")) {
		
		std::string algorithmIndex = InputCommandLineParser::getCmdOption(args, "-subIso");

		if (InputCommandLineParser::cmdOptionExists(args, "-ncan"))
			GlobalConstant::numberOfCandidates = std::stoi(InputCommandLineParser::getCmdOption(args, "-ncan"));
		if (InputCommandLineParser::cmdOptionExists(args, "-dim"))
			GlobalConstant::embeddingDimension = std::stoi(InputCommandLineParser::getCmdOption(args, "-dim"));
		if (InputCommandLineParser::cmdOptionExists(args, "-th"))
			GlobalConstant::distThreshold = std::stof(InputCommandLineParser::getCmdOption(args, "-th"));
		if (InputCommandLineParser::cmdOptionExists(args, "-maxrc"))
			GlobalConstant::G_enoughRecursiveCalls = std::stoi(InputCommandLineParser::getCmdOption(args, "-maxrc"));
		if (InputCommandLineParser::cmdOptionExists(args, "-nresult")) 
			GlobalConstant::G_SUFFICIENT_NUMBER_EMBEDDINGS = std::stoi(InputCommandLineParser::getCmdOption(args, "-nresult"));
		if (InputCommandLineParser::cmdOptionExists(args, "-node")) 
			GlobalConstant::useEmbedding = true;
		if (InputCommandLineParser::cmdOptionExists(args, "-tout")) 
			GlobalConstant::timeOut = std::stof(InputCommandLineParser::getCmdOption(args, "-tout"))*1000;		
		if (InputCommandLineParser::cmdOptionExists(args, "-batch")) 
			GlobalConstant::batchSize = std::stoi(InputCommandLineParser::getCmdOption(args, "-batch"));
		// if (strcmp(algorithmIndex, "TURBOISO") == 0) {
		if (algorithmIndex.compare("TURBOISO") == 0) {
			GlobalConstant::G_RUNNING_OPTION_INDEX = GlobalConstant::RUN_OP_TURBOISO;
		}
		// else if (strcmp(algorithmIndex, "TURBOISO_BOOSTED") == 0) {
		else if (algorithmIndex.compare("TURBOISO_BOOSTED") == 0) {
			GlobalConstant::G_RUNNING_OPTION_INDEX = GlobalConstant::RUN_OP_TURBO_ISO_BOOSTED;
		}
		else if (algorithmIndex.compare("EMBOOST") == 0) {
			GlobalConstant::G_RUNNING_OPTION_INDEX = GlobalConstant::RUN_OP_EMBOOST;
		}
	}
	cout << GlobalConstant::G_enoughRecursiveCalls << endl;
}


void help() {

	cout << "Testing Options: " << endl;
	cout << "-s  Only test the scalability of PCM building. If set, only the query graph file will be optional" << endl;
	cout << "-dg  [datagraphFilename] The datagraph file" << endl;
	cout << "-qg  [querygraphFilename] The querygraph file" << endl;
	cout << "-hg  [hypergraphFilename] The hyerpergraph file" << endl;
	cout << "-cg  [containmentgraphFilename] The boosted containment file" << endl;
	cout << "-out  [outputResultFile] Append the result to the this file" << endl;

#ifdef _DEBUG
	//system("pause"); //@debug
#endif
}
