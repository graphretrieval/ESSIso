#include"PCMBuilder.h"
#include"AdjacenceListsGRAPH_IO.h"
#include<iostream>
#include<algorithm>
#include<set>
#include"GlobalConstant.h"
#include"TimeUtility.h"
#include"Similarity.h"
#include"FindMaximumCommonSubgraph.h"
#include"FindAllMaximalCliques.h"
#include"FindMaximalDisjointConnCliques.h"
#include"MatrixUtility.h"
#include <chrono>

using namespace std;

#define DEBUG

PCMBuilder::PCMBuilder(float ** pTlsGraphMatrix, std::vector<PCM_Node> * pPatternContainmentMap, std::vector<AdjacenceListsGRAPH> * pQueryGraphVector, std::vector<AdjacenceListsGRAPH> * pNewlyGeneratedGraphVector, std::ostream * pResultFile){
	
	tlsGraphMatrix = pTlsGraphMatrix;
	
	queryGraphVector = pQueryGraphVector;
	
	newlyGeneratedGraphVector = pNewlyGeneratedGraphVector;

	patternContainmentMap = pPatternContainmentMap;

	graphIsomorphism = QueryGraphIsomorphismTest();

	subgraphIsomorphism = QuerySubgraphIsomorphismSearch();

	resultFile = pResultFile;
}

PCMBuilder::~PCMBuilder(){
}


void PCMBuilder::execute() {
	/*
	* hide the isomorphic queries
	*/
	hideIsomorphicQueries();

#ifdef DEBUG
	for (size_t i = 0; i < patternContainmentMap->size(); i++) {
		if (patternContainmentMap->at(i).isHidden == true) {
			(*resultFile) << endl << "Hidden Isomorphic Queries: ";
			(*resultFile) << i;
		}
	}
#endif

	/*
	 * subgraph isomorphism is faster than the
	 */
	// cout << "buildContainmentRelations" << endl;
	buildContainmentRelations();

#ifdef DEBUG
	for (size_t i = 0; i < patternContainmentMap->size(); i++) {
		if (patternContainmentMap->at(i).isHidden != true && patternContainmentMap->at(i).descendent.size() > 0) {
			(*resultFile) << endl << "Subgraph Isomorphic Queries: ";
			(*resultFile) << i << " : ";
			for (set<int>::iterator descendentIterator = patternContainmentMap->at(i).descendent.begin(); descendentIterator != patternContainmentMap->at(i).descendent.end(); descendentIterator++) {
				(*resultFile) << *descendentIterator << ", ";
			}
			(*resultFile) << endl;
		}
	}
#endif

	/*
	* Apply the tls threshold and get a new relative matrix with all value true or false
	* Call maximal clique funciton to get groups of queries
	*/
	bool ** tlsRelativeMatrix = new bool*[queryGraphVector->size()];
	for (size_t i = 0; i<queryGraphVector->size(); i++) {
		tlsRelativeMatrix[i] = new bool[queryGraphVector->size()];
	}
	for (size_t i = 0; i<queryGraphVector->size(); i++) {
		tlsRelativeMatrix[i][i] = 0;
		for (size_t j = i+1; j<queryGraphVector->size(); j++) {
			tlsRelativeMatrix[i][j] = tlsGraphMatrix[i][j] > GlobalConstant::G_COMMON_TLS_RATIO ? true : false;
			tlsRelativeMatrix[j][i] = tlsRelativeMatrix[i][j];
		}
	}

#ifdef DEBUG
	// (*resultFile) << endl << "Relative Tls Matrix after threshold : " << GlobalConstant::G_COMMON_TLS_RATIO << endl;
	MatrixUtility::outputBoolMatrix(tlsRelativeMatrix, (*queryGraphVector).size(), (*queryGraphVector).size(), resultFile);
#endif

	/*
	 * based on the relative matrix, we get the query groups.
	 * there are two options to choose. one is to find all the maximal cliques with joint query. the other one is not allow 
	 * joint query. the later option can be faster
	 */
	//findAllMaximalCliques->findMaxclique(tlsRelativeMatrix, &similiarQueryGroups, patternContainmentMap);
	
	FindMaximalDisjointConnCliques::findMaximalDisjointConnCliques(tlsRelativeMatrix, queryGraphVector->size(), similiarQueryGroups);
	for (size_t i = 0; i<queryGraphVector->size(); i++) {
		delete[] tlsRelativeMatrix[i];
	}
	delete [] tlsRelativeMatrix;

#ifdef DEBUG
	printSimilarGroups();
#endif

	/*
	* Based on the trimmed tls matrix, we detect the common graphs
	*/
	// cout << "similiarQueryGroups size " << similiarQueryGroups.size() << endl;
	for (std::vector<vector<int>>::iterator groupIterator = similiarQueryGroups.begin(); groupIterator != similiarQueryGroups.end(); groupIterator++) {
		if (groupIterator->size() < GlobalConstant::G_GOURP_QUERY_CLIQUE_MINI_SIZE) {
			/* for each clique, we restrict the clique size minimum as 3, cause if it is 2, it is only an edge represents 
			 * 2 queries. there is point less to build a relationship like this, as the overhead of saving one graph overweigh the savings shared by only 2
			 */
			continue;
		}
		// cout << "detectCommonSubgraphs " << groupIterator->size() << endl;
		detectCommonSubgraphs(*groupIterator);
	}


	/*
	 * transitive reduction and remove isomorphisic queries
	 */
	// cout << "formatPCM" << endl;
	formatPCM();

#ifdef DEBUG
	showPCM();
#endif
}

void PCMBuilder::hideIsomorphicQueries()
{
	for (size_t i = 0; i < queryGraphVector->size(); i++) {
		for (size_t j = i + 1; j < queryGraphVector->size(); j++) {
			if ((*patternContainmentMap)[j].isHidden == true 
				|| tlsGraphMatrix[i][j] != 1) {
				continue;
			}
			std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

			bool a = false;
			// std::cout << "isGraphIsomorphic time " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() ; 
			try {
				std::cout << "hideIsomorphicQueries" << std::endl;
				a = graphIsomorphism.isGraphIsomorphic(&(*queryGraphVector)[i], &(*queryGraphVector)[j]);
				std::cout << "done" << std::endl;
			}
			// catch(std::runtime_error& e) {
			catch(...) {
				// std::cout << "subgraphIsomorphismSearch " << e.what() << std::endl;
				std::cout << "hideIsomorphicQueries error " << std::endl;
			}
			if ( a ) {
				// std::cout << "Iso WTF " << i << " " << j << std::endl;
				/*
				* Those two query graphs are isomorphic
				*/
				(*patternContainmentMap)[i].equivalentGraph.push_back(j);
				(*patternContainmentMap)[j].isHidden = true;

				for (size_t z = 0; z < queryGraphVector->size(); z++) {
					tlsGraphMatrix[z][j] = 0;
					tlsGraphMatrix[j][z] = 0;
				}

			}
		}
	}
}

void PCMBuilder::buildContainmentRelations()
{

	for (size_t i = 0; i < queryGraphVector->size(); i++) {
		if ((*patternContainmentMap)[i].isHidden == true) {
			// std::cout << "Continue" << std::endl;
			continue;
		} else {
			// std::cout << "Not continue" << std::endl;
		}

		for (size_t j = i + 1; j < queryGraphVector->size(); j++) {
			// std::cout << i << " " << j << std::endl;
			/*
			 * Equivalent Filter
			 */
			if ((*patternContainmentMap)[j].isHidden == true) {
				// std::cout << "Continue" << std::endl;
				continue;
			} else {
				// std::cout << "Not continue" << std::endl;
			}

			AdjacenceListsGRAPH * dataGraph, *queryGraph;
			if ((*queryGraphVector)[i].getNumberOfVertexes() >= (*queryGraphVector)[j].getNumberOfVertexes()) {
				dataGraph = &(*queryGraphVector)[i];
				queryGraph = &(*queryGraphVector)[j];
			}
			else {
				dataGraph = &(*queryGraphVector)[j];
				queryGraph = &(*queryGraphVector)[i];
			}
			// std::cout << "data and query graph are determined" << std::endl;
			/*
			 * Use TLS filter, the similarity of subgraph isomorphic queries is 1
			 */
			// std::cout << "tlsGraphMatrix[i][j]="<<tlsGraphMatrix[i][j]<< std::endl;
			if (tlsGraphMatrix[i][j] != 1) {
				continue;
			}

			/*
			 * Already relationship filter
			 */
			// std::cout << "Another check " << ((*patternContainmentMap)[queryGraph->graphId].descendent.find(dataGraph->graphId) != (*patternContainmentMap)[queryGraph->graphId].descendent.end()) << std::endl;
			if ((*patternContainmentMap)[queryGraph->graphId].descendent.find(dataGraph->graphId) != (*patternContainmentMap)[queryGraph->graphId].descendent.end()) {
				continue;
			}

			vector<vector<int>> mappings;
			try {
				std::cout << "subgraphIsomorphismSearch" << std::endl;
				subgraphIsomorphism.subgraphIsomorphismSearch(dataGraph, queryGraph, &mappings);
				std::cout << "done" << std::endl;
			}
			// catch(std::runtime_error& e) {
			catch(...) {
				// std::cout << "subgraphIsomorphismSearch " << e.what() << std::endl;
				std::cout << "subgraphIsomorphismSearch error " << std::endl;
			}
			// std::cout << mappings.size() << std::endl;
			// subgraphIsomorphism.subgraphIsomorphismSearch(dataGraph, queryGraph, &mappings);
			if (mappings.size() > 0) {
				// std::cout << "Try to insert" << std::endl;
				(*patternContainmentMap)[queryGraph->graphId].descendent.insert(dataGraph->graphId);
				// std::cout << "Try to insert" << std::endl;
				(*patternContainmentMap)[queryGraph->graphId].descendent.insert((*patternContainmentMap)[dataGraph->graphId].descendent.begin(), (*patternContainmentMap)[dataGraph->graphId].descendent.end());
				// std::cout << "Try to insert" << std::endl;
				(*patternContainmentMap)[queryGraph->graphId].containmentRelationshipMappingLists.insert(std::pair<int, vector<vector<int>>>(dataGraph->graphId, mappings));
				// std::cout << "inserted" << std::endl;
			}
		}
	}
	// std::cout << "built" << std::endl;
}

void PCMBuilder::detectCommonSubgraphs(vector<int> & groupQuery) {

	vector<int> nextHierarchy;

	if (groupQuery.size() < 2) {
		return;
	}
	if (groupQuery.size() % 2 == 1) {
		// if the size of query group is odd. we put the last element into the next hierarchy round
		nextHierarchy.push_back(groupQuery[groupQuery.size() - 1]);
	}

	for (size_t pairStart = 0; pairStart + 1 < groupQuery.size(); pairStart += 2) {

		AdjacenceListsGRAPH * pairStartQuery;
		AdjacenceListsGRAPH * pairEndQuery;

		if (groupQuery[pairStart] >= queryGraphVector->size()) {
			pairStartQuery = &(*newlyGeneratedGraphVector)[groupQuery[pairStart] - queryGraphVector->size()];
		}
		else {
			pairStartQuery = &(*queryGraphVector)[groupQuery[pairStart]];
		}
		if (groupQuery[pairStart + 1] >= queryGraphVector->size()) {
			pairEndQuery = &(*newlyGeneratedGraphVector)[groupQuery[pairStart + 1] - queryGraphVector->size()];
		}
		else {
			pairEndQuery = &(*queryGraphVector)[groupQuery[pairStart + 1]];
		}

		if ((*patternContainmentMap)[pairStartQuery->graphId].descendent.find(pairEndQuery->graphId) != (*patternContainmentMap)[pairStartQuery->graphId].descendent.end()
			|| (*patternContainmentMap)[pairEndQuery->graphId].descendent.find(pairStartQuery->graphId) != (*patternContainmentMap)[pairEndQuery->graphId].descendent.end()) {
			//If those two graph are isomphic or something and computed earlier. we don't have to recompute it again
			continue;
		}

		AdjacenceListsGRAPH mcsGraph;
		// cout << "computeMaximumCommonSubgraph " << pairStartQuery->getNumberOfVertexes() << " " << pairEndQuery->getNumberOfVertexes() << endl;
		FindMaximumCommonSubgraph::computeMaximumCommonSubgraph(pairStartQuery, pairEndQuery, &mcsGraph);
		// cout << "end " << mcsGraph.getNumberOfVertexes() << endl;
		if (mcsGraph.getNumberOfVertexes() == 0) {
			continue;
		}
		/*
		 * Decide whether this mcs should be added to the pcm
		 */
		mcsGraph.buildTLSequence();
		mcsGraph.buildLabelVertexList();
		/*
		 * use a isomorphic graph to carry on the mcs graph if there is an isomorphic query
		 */
		AdjacenceListsGRAPH * isomorphicGraph = NULL;
		for (size_t i = 0; i < queryGraphVector->size(); i++) {
			if (Similarity::tlsSimilarity(&mcsGraph, &(*queryGraphVector)[i]) == 1
				&& graphIsomorphism.isGraphIsomorphic(&mcsGraph, &(*queryGraphVector)[i])) {

				isomorphicGraph = &(*queryGraphVector)[i];
				break;
			}
		}
		for (size_t i = 0; i < newlyGeneratedGraphVector->size(); i++) {
			if (Similarity::tlsSimilarity(&mcsGraph, &(*newlyGeneratedGraphVector)[i]) == 1
				&& graphIsomorphism.isGraphIsomorphic(&mcsGraph, &(*newlyGeneratedGraphVector)[i])) {

				isomorphicGraph = &(*newlyGeneratedGraphVector)[i];
				break;
			}
		}

		
		bool foundedIsomorphic = false;
		if (isomorphicGraph == NULL) {
			foundedIsomorphic = true;
			/*
			* Can be added to the PCM. build rest indexes and add it to the newly generated vector
			*/
			mcsGraph.graphId = queryGraphVector->size() + newlyGeneratedGraphVector->size();

			mcsGraph.buildVertexLabelEdgeList();
			mcsGraph.buildVertexLabelVertexList();
			mcsGraph.buildComboGraph(GlobalConstant::G_MAXIMUM_WIDTH_COMBO);

			PCM_Node pcm_node(mcsGraph.graphId, true);
			patternContainmentMap->push_back(pcm_node);

			/*
			* this is a new mcs. We add it into the next hierarchy
			*/
			nextHierarchy.push_back(mcsGraph.graphId);

			/*
			* there is no isomorphic graph in the pcm, thus we can use this newly mcsGraph
			*/
			isomorphicGraph = &mcsGraph;
		}
		if (isomorphicGraph->graphId != pairStartQuery->graphId) {
			if ((*patternContainmentMap)[isomorphicGraph->graphId].descendent.find(pairStartQuery->graphId) == (*patternContainmentMap)[isomorphicGraph->graphId].descendent.end()) {
				vector<vector<int>> mappings;
				subgraphIsomorphism.subgraphIsomorphismSearch(pairStartQuery, isomorphicGraph, &mappings);
				/*
				* set the mapping instances from this mcs's parents to the mcs
				*/
				if (mappings.size() > 0) {
					(*patternContainmentMap)[isomorphicGraph->graphId].descendent.insert(pairStartQuery->graphId);
					(*patternContainmentMap)[isomorphicGraph->graphId].containmentRelationshipMappingLists.insert(std::pair<int, vector<vector<int>>>(pairStartQuery->graphId, mappings));
				}
			}
		}
		if (isomorphicGraph->graphId != pairEndQuery->graphId) {
			if ((*patternContainmentMap)[isomorphicGraph->graphId].descendent.find(pairEndQuery->graphId) == (*patternContainmentMap)[isomorphicGraph->graphId].descendent.end()) {
				vector<vector<int>> mappings;
				subgraphIsomorphism.subgraphIsomorphismSearch(pairEndQuery, &mcsGraph, &mappings);
				if (mappings.size() > 0) {
					(*patternContainmentMap)[isomorphicGraph->graphId].descendent.insert(pairEndQuery->graphId);
					(*patternContainmentMap)[isomorphicGraph->graphId].containmentRelationshipMappingLists.insert(std::pair<int, vector<vector<int>>>(pairEndQuery->graphId, mappings));
				}
			}
		}
		
		if (foundedIsomorphic) {
			/*
			* add this newly generated mcs into the query vector and the pcm
			* don't update the newlyGeneratedGraphVector if you still you startQuery or endQuery, as the address they pointed can be changed
			*/
			newlyGeneratedGraphVector->push_back(mcsGraph);
		}


	}

	/*
	 * start the next hierarchy iteration
	 */
	detectCommonSubgraphs(nextHierarchy);
}



void PCMBuilder::printSimilarGroups() {
	for (std::vector<vector<int>>::iterator similarQueryGroupIterator = similiarQueryGroups.begin(); similarQueryGroupIterator != similiarQueryGroups.end(); similarQueryGroupIterator++) {
		(*resultFile) << "Clique size: " << similarQueryGroupIterator->size() << endl;
		for (vector<int>::iterator groupMemIter = similarQueryGroupIterator->begin(); groupMemIter != similarQueryGroupIterator->end(); groupMemIter++) {
			(*resultFile) << " " << *groupMemIter;
		}
		(*resultFile) << endl;
	}
	(*resultFile) << "TLS: Total cliques: " << similiarQueryGroups.size() << endl;
}

/*
 * 1. remove not neccsseary nodes. such as the node that contains only on child
 * 2. Do the transitive reduction
 * 3. Set up mapping vertices set helper variables
 */
void PCMBuilder::formatPCM(){
	//1 . remove uselss node
	for (unsigned int x = 0; x < (*patternContainmentMap).size(); x++) {
		if ((*patternContainmentMap)[x].descendent.size() < GlobalConstant::G_PCM_MINIMUM_CHILDREN_NUMBER){
			if (x < queryGraphVector->size()) {
				// this is a original query, we need compute it anyway. but not going to save its results
				(*patternContainmentMap)[x].descendent = std::set<int>();
			}
			else {
				(*patternContainmentMap)[x].isHidden = true;
				(*patternContainmentMap)[x].containmentRelationshipMappingLists = std::map<int, std::vector<std::vector<int>>>();
				(*patternContainmentMap)[x].containmentRelationshipMappingSet = std::map<int, std::set<int>>();
				(*patternContainmentMap)[x].descendent = std::set<int>();
			}
		}
	}

	//2. Do the transitive reduction
	bool onlyDirectChildren;
	for(unsigned int x=0; x<(*patternContainmentMap).size();x++){
		if((*patternContainmentMap)[x].isHidden){				
			continue;
		}
		for(std::set<int>::iterator zIterator = (*patternContainmentMap)[x].descendent.begin(); zIterator != (*patternContainmentMap)[x].descendent.end(); zIterator++){
			if((*patternContainmentMap)[*zIterator].isHidden){				
				continue;
			}

			onlyDirectChildren = true;
			for(std::set<int>::iterator yIterator = (*patternContainmentMap)[x].descendent.begin(); yIterator != (*patternContainmentMap)[x].descendent.end(); yIterator++){
				if(*zIterator == *yIterator || (*patternContainmentMap)[*yIterator].isHidden){
					continue;
				}
				if((*patternContainmentMap)[*yIterator].descendent.find(*zIterator) != (*patternContainmentMap)[*yIterator].descendent.end()){
					onlyDirectChildren = false;
					break;
				}
			}
			if(onlyDirectChildren){
				(*patternContainmentMap)[x].children.push_back(*zIterator);
				(*patternContainmentMap)[*zIterator].parent.push_back(x);
			}
		}
	}

	//2. Set up mapping vertices set helper variables
	for(std::vector<PCM_Node>::iterator pcmNodeIterator = patternContainmentMap->begin(); pcmNodeIterator != patternContainmentMap->end(); pcmNodeIterator++){
		for(std::map<int, std::vector<std::vector<int>>>::iterator mappingListsIterator = pcmNodeIterator->containmentRelationshipMappingLists.begin(); mappingListsIterator != pcmNodeIterator->containmentRelationshipMappingLists.end(); mappingListsIterator++){
			std::set<int> mappingQueryVerticesSet;
			for(vector<vector<int>>::iterator mappingListIterator = mappingListsIterator->second.begin(); mappingListIterator != mappingListsIterator->second.end(); mappingListIterator++){
				for(vector<int>::iterator mappingVertexIterator = mappingListIterator->begin(); mappingVertexIterator != mappingListIterator->end(); mappingVertexIterator++){
					mappingQueryVerticesSet.insert(*mappingVertexIterator);	
				}
			}
			pcmNodeIterator->containmentRelationshipMappingSet.insert(std::pair<int, std::set<int>>(mappingListsIterator->first, mappingQueryVerticesSet));	
		}
	}
}


void printDetail(PCM_Node & pcmNode, std::vector<PCM_Node> * patternContainmentMap, std::ostream * resultFile){
	if(pcmNode.equivalentGraph.size() > 0){
		(*resultFile) <<"Equivalent graph: ";
		for(std::vector<int>::iterator iterator = pcmNode.equivalentGraph.begin(); iterator != pcmNode.equivalentGraph.end();iterator++){
			(*resultFile) <<(*iterator)<<((*patternContainmentMap)[*iterator].isGeneratedQueryGraph == false ? "(*)" : "(N)")<<",";
		}
		(*resultFile) <<endl;
	}
	if(pcmNode.descendent.size() > 0){
		(*resultFile) <<"Descendent graph: ";
		for(std::set<int>::iterator iterator = pcmNode.descendent.begin(); iterator != pcmNode.descendent.end();iterator++){
			(*resultFile) <<(*iterator)<<((*patternContainmentMap)[*iterator].isGeneratedQueryGraph == false ? "(*)" : "(N)")<<",";
		}
		(*resultFile) <<endl;
	}
	if(pcmNode.children.size() > 0){
		(*resultFile) <<"Children graph: ";
		for(std::vector<int>::iterator iterator = pcmNode.children.begin(); iterator != pcmNode.children.end();iterator++){
			(*resultFile) <<(*iterator)<<((*patternContainmentMap)[*iterator].isGeneratedQueryGraph == false ? "(*)" : "(New)")<<",";
		}
		(*resultFile) <<endl;
	}
	if(pcmNode.parent.size() > 0){
		(*resultFile) <<"Parent graph: ";
		for(std::vector<int>::iterator iterator = pcmNode.parent.begin(); iterator != pcmNode.parent.end();iterator++){
			(*resultFile) <<(*iterator)<<((*patternContainmentMap)[*iterator].isGeneratedQueryGraph == false ? "(*)" : "(New)")<<",";
		}
		(*resultFile) <<endl;
	}
}

void PCMBuilder::showPCM(){

	int numberOfPcmEdges = 0;
	int numberOfEquivalentNodes = 0;
	
	for (std::vector<PCM_Node>::iterator pcmNodeIterator = patternContainmentMap->begin(); pcmNodeIterator != patternContainmentMap->end(); pcmNodeIterator++) {
		if (!pcmNodeIterator -> isHidden) {
			numberOfPcmEdges += pcmNodeIterator->children.size();
			numberOfEquivalentNodes += pcmNodeIterator->equivalentGraph.size();
			(*resultFile) << endl << "Graph" << pcmNodeIterator->graphId << endl;
			printDetail(*pcmNodeIterator, patternContainmentMap, &cout);
		}
	}

	int newlyCommonGraphs = patternContainmentMap->size() - queryGraphVector->size();

	(*resultFile) << endl << "****PCM Information: " << endl;
	(*resultFile) << "PCM edges: "<< numberOfPcmEdges << endl;
	(*resultFile) << "PCM newly nodes : " << newlyCommonGraphs << endl;
	(*resultFile) << "PCM query nodes : " << queryGraphVector->size() << endl;
	(*resultFile) << "PCM Euivalent nodes : " << numberOfEquivalentNodes << endl;
}