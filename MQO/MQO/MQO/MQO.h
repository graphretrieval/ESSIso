#ifndef MQO_DEF_H
#define MQO_DEF_H

#include"AdjacenceListsGraph.h"
#include"AdjacenceListsGRAPH_IO.h"
#include"TLSGraph.h"
#include"MQO.h"
#include"PCMBuilder.h"
#include"ComboLinkedLists.h"
#include<vector>
#include<iostream>
#include"GlobalConstant.h"
#include"ExecutionOrderNode.h"
#include"JointGraph.h"
#include"TurboIso.h"
#include"TurboIsoMQO.h"
#include"TurboIsoBoosted.h"
#include"TurboIsoBoostedMQO.h"

class MQO {

private:

	/*
	* service functions
	*/

	std::ofstream * resultFile;

private:
	/*
	* datasets
	*/
	std::vector<AdjacenceListsGRAPH> * queryGraphVector;

	AdjacenceListsGRAPH * dataGraph;

	AdjacenceListsGRAPH * queryGraph;

private:
	/*
	* used to contain the containment relationships of the queries and newly generated graphs
	*/
	std::vector<PCM_Node> patternContainmentMap;

	/*
	* used to store the Triple Label Sequence Matrix
	*/
	float ** tlsGraphMatrix;

	/*
	* used to store the all the newly generated graphs
	*/
	std::vector<AdjacenceListsGRAPH> newlyGeneratedGraphVector;

private:

	/*
	 * the query execution order based on pcm
	 */
	std::vector<ExecutionOrderNode> hybridExecutionOrder;


private:
	/*
	 * used to save the embedding caches
	 * points to the head of each linked list
	 * this is an array where each element is an entry for the cache of a query
	 */
	std::vector<std::vector<CacheEmbeddingNode *>> resultCaches;

	/*
	 * Used to save the number of founded embeddings
	 */
	std::vector<int> numberOfEmbeddings;

private:

	JointGraph jointGraph;

private:

	TurboIso * turboIso;

	TurboIsoMQO * turboIsoMQO;

	TurboIsoBoosted * turboIsoBoosted;

	TurboIsoBoostedMQO * turboIsoBoostedMQO;

public:

	MQO();
	MQO(std::vector<AdjacenceListsGRAPH> * dataGraphVector, std::vector<AdjacenceListsGRAPH> * pQueryGraphVector, std::ofstream * pResultFile);
	~MQO();

	void buildPCM();

	void buildQueryExecutionOrder();

	void orederedQueryProcessing();

private:
	
	void buildJointGraph(ExecutionOrderNode * orderNode);

};



#endif 