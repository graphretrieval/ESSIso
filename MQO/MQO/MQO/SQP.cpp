#include"SQP.h"
#include"TimeUtility.h"
#include"GlobalConstant.h"
#include<queue>


SQP::SQP(){}

SQP::SQP(std::vector<AdjacenceListsGRAPH>* dataGraphVector, std::vector<AdjacenceListsGRAPH>* pQueryGraphVector, std::ofstream * pResultFile)
{

	if (dataGraphVector->size() > 0) {
		dataGraph = &(*dataGraphVector)[0];
	}
	totalCall = 0;

	queryGraphVector = pQueryGraphVector;

	resultFile = pResultFile;

	/*
	 * Subgraph Isomorphism algorithms
	 */
	turboIsoBoosted = new TurboIsoBoosted(dataGraph, &numberOfEmbeddings, NULL);

	turboIso = new TurboIso(dataGraph, &numberOfEmbeddings, NULL);

	emBoost = new EmBoost(dataGraph, &numberOfEmbeddings, NULL);
}

SQP::~SQP()
{
	numberOfEmbeddings = std::vector<int>();

	delete turboIsoBoosted;

	delete turboIso;

	delete emBoost;
}


void SQP::queryProcessing(){
	/*
	 * initialize cache embedding for all the query graphs
	 */
	for(unsigned int i=0; i<queryGraphVector->size(); i++){
		numberOfEmbeddings.push_back(0);
	}

	for(unsigned int i=0; i<queryGraphVector->size(); i++){
		TimeUtility ttt;
		ttt.StartCounterMill();

		queryGraph =  &(*queryGraphVector)[i];
		double time =  0;
		switch (GlobalConstant::G_RUNNING_OPTION_INDEX) {
		case GlobalConstant::RUN_OP_TURBOISO:
			turboIso->setParameters(queryGraph, false);
			try {
				turboIso->execute();
			}
			catch(std::runtime_error& e) {
				std::cout << e.what() << std::endl;
			}
			// turboIso->execute();
			totalCall += turboIso->recursiveCallsNo;
			time =  ttt.GetCounterMill();
			(*resultFile)<< time << " " << turboIso->recursiveCallsNo << " " << numberOfEmbeddings[i]<<endl;
			break;
		case GlobalConstant::RUN_OP_TURBO_ISO_BOOSTED:
			turboIsoBoosted->setParameters(queryGraph, false);
			turboIsoBoosted->execute();
			break;
		case GlobalConstant::RUN_OP_EMBOOST:
			emBoost->setParameters(queryGraph, false);
			emBoost->execute();
			totalCall += emBoost->recursiveCallsNo;
			break;
		}
		cout << " Time Cost " << time << endl;
		cout << "Finish One Query: " << i << " Embedding founded " << numberOfEmbeddings[i] << endl;

	}
	if (GlobalConstant::G_RUNNING_OPTION_INDEX ==GlobalConstant::RUN_OP_TURBOISO )
		cout << turboIso->totalTime << endl;
}