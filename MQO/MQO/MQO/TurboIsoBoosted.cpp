#include"MathUtility.h"
#include<iostream>

#include<climits>
#include"TurboIsoBoosted.h"
#include"AdjacenceListsGRAPH_BOOST.h"
#include"GlobalConstant.h"
#include<float.h>

using namespace std;



TurboIsoBoosted::TurboIsoBoosted(AdjacenceListsGRAPH * pHyperGraph, std::vector<int> * pNumberOfEmbeddings, std::vector<std::vector<CacheEmbeddingNode *>> * pResultCaches) {
	hyperGraph = pHyperGraph;
	numberOfEmbeddings = pNumberOfEmbeddings;
	resultCaches = pResultCaches;

	embedding = NULL;
}

void TurboIsoBoosted::setParameters(AdjacenceListsGRAPH * pQueryGraph, bool pNeedToSaveCache) {

	queryGraph = pQueryGraph;
	needFormatCache = pNeedToSaveCache;

	if (embedding != NULL) {
		delete embedding;
		embedding = NULL;
	}
	embedding = new int[queryGraph->getNumberOfVertexes()];
	for (int i = 0; i < queryGraph->getNumberOfVertexes(); i++) {
		embedding[i] = -1;
	}

	inverseEmbedding = std::map<int, std::stack<int>>();

	needReturn = false;
	mappedQueryVertexSize = 0;

	necTree = std::vector<NECNode>();
	startingQueryVertexCandidates = std::queue<int>();
	scRootCandidates = std::map<int, vector<int>* >();
}


void TurboIsoBoosted::execute() {

	startQueryVertex = chooseStartVertex();
	if (startQueryVertex == -1) {
		return;
	}
	/*
	 * also we cannot allow choosing starting vertex if you choose the queries starting from vertex with many labels. 
	 * Because there are only a few vertices having many labels
	 */
	//@debug
	startQueryVertex = 1;

	startQueryVertexAddress = queryGraph->getVertexAddressByVertexId(startQueryVertex); 	//@debug

	/*
	* We can only apply dynamic loading strategy to the starting vertex candidates
	*/
	filterCandidates();
	rewriteToNecTree();


	/*
	* For turboIsoBoosted_MQO. we won't compute the DRT because we don't allow the starting Vertex as we choose the query specifically
	*/
	for (vector<int>::iterator candidateIterator = scRootCandidates[startQueryVertex]->begin(); candidateIterator != scRootCandidates[startQueryVertex]->end(); candidateIterator++) {
		startingQueryVertexCandidates.push(*candidateIterator);
	}
	
	while (!startingQueryVertexCandidates.empty()) {
		if (needReturn) {
			break;
		}

		int nextCandidate = startingQueryVertexCandidates.front();
		startingQueryVertexCandidates.pop();

		vector<int> subRegionCandidates;
		subRegionCandidates.push_back(nextCandidate);

		// explore the ER start from the root of the nectree
		CR = map<std::pair<int, int>, set<int>>();

		if (exploreCR(0, -1, subRegionCandidates) == false) {
			continue;
		}

		/*
		* Computing the vertication order based on the saved CR
		*/
		computeMatchingOrder();

		updateState(startQueryVertexAddress, nextCandidate);
		subgraphSearch();
		restoreState(startQueryVertexAddress, nextCandidate);

		/*
		* Apply dynamic loading strategy to the candidates of the starting query vertex
		*/
		dynamicLoadCandidates(nextCandidate);
	}
}



/**
* The candidate filter for turboIso only compute the candidates for the start vertex
*/
void TurboIsoBoosted::filterCandidates() {

	map<int, vector<int>> * rootLabelDataVertexList = hyperGraph->getLabelContainmentRootList();

	int queryVertexLabel = queryGraph->getVertexAddressByVertexId(startQueryVertex)->label;

	scRootCandidates.insert(std::pair<int, std::vector<int> *>(startQueryVertex, &rootLabelDataVertexList->find(queryVertexLabel)->second));

}

void TurboIsoBoosted::updateState(AdjacenceListsGRAPH::Vertex * u, int v) {
	embedding[u->id] = v;

	mappedQueryVertexSize++;
	if (inverseEmbedding.find(v) == inverseEmbedding.end()) {
		stack<int> vertexList;
		vertexList.push(u->id);
		inverseEmbedding.insert(std::pair<int, stack<int>>(v, vertexList));
	}
	else {
		inverseEmbedding.find(v)->second.push(u->id);
	}

}

void TurboIsoBoosted::restoreState(AdjacenceListsGRAPH::Vertex * u, int v) {
	embedding[u->id] = -1;
	mappedQueryVertexSize--;

	std::map<int, std::stack<int>>::iterator Global_stack_Iterator = inverseEmbedding.find(v);
	Global_stack_Iterator->second.pop();

	if (Global_stack_Iterator->second.size() == 0) {
		inverseEmbedding.erase(v);
	}
}

TurboIsoBoosted::NECNode * TurboIsoBoosted::nextQueryVertex()
{
	for (size_t i = 0; i < queryMatchingSuquence.size(); i++) {
		if (embedding[necTree[queryMatchingSuquence[i]].vertexId] == -1) {
			return 	&necTree[queryMatchingSuquence[i]];
		}
	}
}

void TurboIsoBoosted::subgraphSearch() {
	// For the completeness of MQO, we have to let the algorithm run until find all the embeddings. 
	/*if (recursiveCallsNo++ >= GlobalConstant::G_enoughRecursiveCalls) {
		needReturn = true;
		return;
	}*/

	if (mappedQueryVertexSize == queryGraph->getNumberOfVertexes()) {

		(*numberOfEmbeddings)[queryGraph->graphId] ++;
		if (needFormatCache) {
			ComboLinkedLists::addEmbeddingToCache(resultCaches, queryGraph, embedding);
		}

		return;
	}

	NECNode  * nectree_u = nextQueryVertex();

	map<std::pair<int, int>, set<int>>::iterator candidateListIterator = CR.find(std::pair<int, int>(nectree_u->id, embedding[necTree[nectree_u->parent].vertexId]));
	if (candidateListIterator == CR.end()) {
		return;
	}
	// start iteration candidates
	for (set<int>::iterator candidateIterator = candidateListIterator->second.begin(); candidateIterator != candidateListIterator->second.end(); candidateIterator++) {
		if (needReturn) {
			return;
		}

		// For the completeness of MQO, we have to let the algorithm run until find all the embeddings. 
		/*
		if ((*numberOfEmbeddings)[queryGraph->graphId] >= GlobalConstant::G_SUFFICIENT_NUMBER_EMBEDDINGS) {
			break; // only calculate 1000 embeddings for each query graph
		}
		*/

		// isJoinable Filtering
		if (!isJoinable(queryGraph->getVertexAddressByVertexId(nectree_u->vertexId), hyperGraph->getVertexAddressByVertexId(*candidateIterator))) {
			continue;
		}

		updateState(queryGraph->getVertexAddressByVertexId(nectree_u->vertexId), *candidateIterator);
		subgraphSearch();
		restoreState(queryGraph->getVertexAddressByVertexId(nectree_u->vertexId), *candidateIterator);
	}


}

bool TurboIsoBoosted::isJoinable(AdjacenceListsGRAPH::Vertex * u, AdjacenceListsGRAPH::Vertex * v) {

	for (AdjacenceListsGRAPH::link t = u->adj; t != 0; t = t->next) {

		if (embedding[t->v] != -1) {
			if (embedding[t->v] == v->id) {
				/*
				* If two connected query vertices are trying to match to the same hypervertex, we need to make sure this hypervertex is a clique
				*/
				if (v->isClique == 2) {
					return false;
				}
			}
			else if (!hyperGraph->edge(v->id, embedding[t->v])) {
				return false;
			}
		}
		/*
		* Check whether the data vertex has already been matched, If it is, we need to make sure it cannot be matched more than the data vertices it has
		*/
		std::map<int, std::stack<int>>::iterator inverseIterator = inverseEmbedding.find(v->id);
		if (inverseIterator != inverseEmbedding.end()) {
			if (inverseIterator->second.size() + 1 > v->sEquivalentVertexList.size()) {
				return false;
			}
		}
	}
	return true;
}


/*
*
*/
void TurboIsoBoosted::computeMatchingOrder() {
	queryMatchingSuquence = std::vector<int>();
	// set the start vertex
	float * queryVertexScore = new float[necTree.size()];
	for (size_t queryVertexIndex = 0; queryVertexIndex<necTree.size(); queryVertexIndex++) {
		queryVertexScore[queryVertexIndex] = 0;
	}

	for (map<std::pair<int, int>, set<int>>::iterator crIterator = CR.begin(); crIterator != CR.end(); crIterator++) {
		queryVertexScore[crIterator->first.first] += crIterator->second.size();
	}

	for (size_t queryVertexIndex = 0; queryVertexIndex<necTree.size(); queryVertexIndex++) {
		// devide the no-tree edges
		int numberOfNoTreeEdges;
		if (necTree[queryVertexIndex].parent != -1) {
			numberOfNoTreeEdges = queryGraph->getVertexByVertexId(necTree[queryVertexIndex].vertexId).inDegree - necTree[queryVertexIndex].childList.size() - 1;
		}
		else {
			numberOfNoTreeEdges = queryGraph->getVertexByVertexId(necTree[queryVertexIndex].vertexId).inDegree - necTree[queryVertexIndex].childList.size();
		}
		devideNoTreeEdges(queryVertexIndex, numberOfNoTreeEdges + 1, queryVertexScore);
	}

	vector<int> nextVertex;
	for (vector<int>::iterator childIterator = necTree[0].childList.begin(); childIterator != necTree[0].childList.end(); childIterator++) {
		nextVertex.push_back(*childIterator);
	}
	getOrderByBFSScore(0, queryVertexScore, nextVertex);


	delete[] queryVertexScore;
}


void TurboIsoBoosted::devideNoTreeEdges(int u, int numberOfNoTreeEdges, float * queryVertexScore) {
	queryVertexScore[u] /= numberOfNoTreeEdges;
	for (vector<int>::iterator childIterator = necTree[u].childList.begin(); childIterator != necTree[u].childList.end(); childIterator++) {
		devideNoTreeEdges(*childIterator, numberOfNoTreeEdges, queryVertexScore);
	}
}

void TurboIsoBoosted::getOrderByBFSScore(int u, float * queryVertexScore, vector<int> & nextVertex) {

	queryMatchingSuquence.push_back(u);

	if (nextVertex.size() == 0) {
		return;
	}

	float mimimum = FLT_MAX;
	int mimimumVertex = 1;
	int vertexNextId = -1;

	for (size_t i = 0; i < nextVertex.size(); i++) {
		if (queryVertexScore[nextVertex[i]] == -1) {
			continue;
		}
		else {
			if (queryVertexScore[nextVertex[i]] < mimimum) {
				mimimumVertex = nextVertex[i];
				vertexNextId = i;
				mimimum = queryVertexScore[nextVertex[i]];
			}
		}
	}
	queryVertexScore[mimimumVertex] = -1;
	if (vertexNextId != -1) {
		nextVertex.erase(nextVertex.begin() + vertexNextId);
	}


	for (vector<int>::iterator childIterator = necTree[mimimumVertex].childList.begin(); childIterator != necTree[mimimumVertex].childList.end(); childIterator++) {
		nextVertex.push_back(*childIterator);
	}
	getOrderByBFSScore(mimimumVertex, queryVertexScore, nextVertex);
}


bool TurboIsoBoosted::exploreCR(int u, int parentMappedNecTree, vector<int> & subRegionCandidates) {

	for (size_t vmIndex = 0; vmIndex < subRegionCandidates.size(); vmIndex++) {

		bool matched = true;

		for (vector<int>::iterator childIterator = necTree[u].childList.begin(); childIterator != necTree[u].childList.end(); childIterator++) {

			vector<int> adjChildAdj;


			AdjacenceListsGRAPH::Vertex dataGraphChild = hyperGraph->getVertexByVertexId(subRegionCandidates[vmIndex]);
			map<int, vector<int>>::iterator childLabelList = dataGraphChild.labelVertexList.find(necTree[*childIterator].label);

			if (childLabelList != dataGraphChild.labelVertexList.end()) {
				for (vector<int>::iterator childLabelItem = childLabelList->second.begin(); childLabelItem != childLabelList->second.end(); childLabelItem++) {
					// TODO, no need extra index
					//if (!AdjacenceListsGRAPH_BOOST::degreeFilter(queryGraph, hyperGraph, necTree[*childIterator].vertexId, *childLabelItem)) {
					//	continue;
					//}

					adjChildAdj.push_back(*childLabelItem);
				}
			}


			/*
			* A important modification for the exploreCR for BoostIso goes here. We need to add itself here to suit for the dynamic candidate loading strategy
			*/
			if (parentMappedNecTree == -1) {
				if (AdjacenceListsGRAPH_BOOST::degreeFilter(queryGraph, hyperGraph, necTree[*childIterator].vertexId, subRegionCandidates[vmIndex])) {
					adjChildAdj.push_back(subRegionCandidates[vmIndex]);
				}
			}

			if (exploreCR(necTree[*childIterator].id, subRegionCandidates[vmIndex], adjChildAdj) == false) {
				/*
				* If one of its child cannot match under this subregion
				* 1. stop the process, and the elements at "vmIndex" won't be added
				* Two reasons not to delete anything.
				* 1. No need to, we can allow some rubbish data in the memory while no need to bother to clean them before the computing of this query is done.
				* 2. In some special cases, you cannot delete it. A grand-relationship will cause error.
				*/
				matched = false;
				break;
			}
		}
		if (matched == false) {
			continue;
		}
		map<std::pair<int, int>, set<int>>::iterator  CRIterator = CR.find(std::pair<int, int>(u, parentMappedNecTree));
		if (CRIterator == CR.end()) {
			set<int> CRVertices;
			CRVertices.insert(subRegionCandidates[vmIndex]);
			CR.insert(std::pair<std::pair<int, int>, set<int>>(std::pair<int, int>(u, parentMappedNecTree), CRVertices));
		}
		else {
			CRIterator->second.insert(subRegionCandidates[vmIndex]);
		}
	}

	if (CR.find(std::pair<int, int>(u, parentMappedNecTree)) == CR.end()) {
		return false;
	}

	return true;
}




/*
* Dynamically append candidates based on the (s and qd) containment relationships
*/
void TurboIsoBoosted::dynamicLoadCandidates(int dataVertexId) {

	/*
	* For each just mapped vertex
	*/
	AdjacenceListsGRAPH::Vertex* mappedVertex = (AdjacenceListsGRAPH::Vertex*) hyperGraph->getVertexAddressByVertexId(dataVertexId);

	/*
	* 1. We add its direct qdContained candiates into dynamic candidate list
	*/
	for (std::vector<int>::iterator qdChildIterator = mappedVertex->qdContainedVertexList.begin(); qdChildIterator != mappedVertex->qdContainedVertexList.end(); qdChildIterator++) {
		AdjacenceListsGRAPH::Vertex* qdChildVertex = (AdjacenceListsGRAPH::Vertex*) hyperGraph->getVertexAddressByVertexId(*qdChildIterator);
		if (qdChildVertex->qdeEquivalentHidden == false) {
			qdChildVertex->qdSpongeIndegree--;
			if (qdChildVertex->qdSpongeIndegree == 0) {
				if (!AdjacenceListsGRAPH_BOOST::degreeFilter(queryGraph, hyperGraph, startQueryVertex, *qdChildIterator)) {
					continue;
				}
				startingQueryVertexCandidates.push(*qdChildIterator);
				qdChildVertex->qdSpongeIndegree = qdChildVertex->qdIndegree;
			}
		}
	}

	/*
	* 2. We add its direct sContained candiates into dynamic candidate list
	*/
	for (std::vector<int>::iterator sChildIterator = mappedVertex->sContainedVertexList.begin(); sChildIterator != mappedVertex->sContainedVertexList.end(); sChildIterator++) {
		AdjacenceListsGRAPH::Vertex* sChildVertex = hyperGraph->getVertexAddressByVertexId(*sChildIterator);
		sChildVertex->spongeIndegree--;
		if (sChildVertex->spongeIndegree == 0) {
			if (AdjacenceListsGRAPH_BOOST::degreeFilter(queryGraph, hyperGraph, startQueryVertex, *sChildIterator)) {
				startingQueryVertexCandidates.push(*sChildIterator);
			}
			sChildVertex->spongeIndegree = sChildVertex->sIndegree;
		}
	}

	/*
	* 3. For each of its qde quivalent vertex, we need to add their sContained candiates into dynamic candidate list
	*/
	for (std::vector<int>::iterator qeChildIterator = mappedVertex->qdEquivalentVertexList.begin(); qeChildIterator != mappedVertex->qdEquivalentVertexList.end(); qeChildIterator++) {
		AdjacenceListsGRAPH::Vertex* qeChildVertex = hyperGraph->getVertexAddressByVertexId(*qeChildIterator);
		for (std::vector<int>::iterator qe_s_childIterator = qeChildVertex->sContainedVertexList.begin(); qe_s_childIterator != qeChildVertex->sContainedVertexList.end(); qe_s_childIterator++) {
			AdjacenceListsGRAPH::Vertex* qe_s_childVertex = hyperGraph->getVertexAddressByVertexId(*qe_s_childIterator);
			qe_s_childVertex->spongeIndegree--;
			if (qe_s_childVertex->spongeIndegree == 0) {
				if (AdjacenceListsGRAPH_BOOST::degreeFilter(queryGraph, hyperGraph, startQueryVertex, *qe_s_childIterator)) {
					startingQueryVertexCandidates.push(*qe_s_childIterator);
				}
				qe_s_childVertex->spongeIndegree = qe_s_childVertex->sIndegree;
			}
		}
	}

}


/*
* Choose the starting query vertex. This process is based on the frequence information for each label
*/
int TurboIsoBoosted::chooseStartVertex() {

	std::vector<AdjacenceListsGRAPH::Vertex>::iterator queryVertexIterator;

	std::map<int, vector<int>>* labelVertexList = hyperGraph->getLabelVertexList();
	std::map<int, vector<int>>::iterator labelVertexIterator;

	vector<float> rankScore;
	for (queryVertexIterator = queryGraph->getVertexList()->begin(); queryVertexIterator != queryGraph->getVertexList()->end(); queryVertexIterator++) {
		if (queryVertexIterator->inDegree != 0) {
			labelVertexIterator = labelVertexList->find(queryVertexIterator->label);
			if (labelVertexIterator != labelVertexList->end()) {
				rankScore.push_back((float)labelVertexIterator->second.size() / queryVertexIterator->inDegree);
			}
			else {
				return -1;
			}
		}
	}
	// minimum query vertex number is 2
	int topThree[3];
	for (int i = 0; i<3; i++) {
		topThree[i] = -1;
	}
	float miniScore  = FLT_MAX;
	for (size_t i = 0; i<rankScore.size(); i++) {
		if (rankScore[i] < miniScore) {
			miniScore = rankScore[i];
			topThree[0] = i;
		}
	}
	miniScore  = FLT_MAX;
	for (size_t i = 0; i<rankScore.size(); i++) {
		if (i != topThree[0]) {
			if (rankScore[i] < miniScore) {
				miniScore = rankScore[i];
				topThree[1] = i;
			}
		}
	}
	miniScore  = FLT_MAX;
	if (rankScore.size() >= 3) {
		for (size_t i = 0; i<rankScore.size(); i++) {
			if (i != topThree[0] && i != topThree[1]) {
				if (rankScore[i] < miniScore) {
					miniScore = rankScore[i];
					topThree[2] = i;
				}
			}
		}
	}
	int topThreeNumberOfCandidates[3];
	int us;
	int miniTopThree = INT_MAX;
	for (int i = 0; i<3; i++) {
		topThreeNumberOfCandidates[i] = 0;
		if (topThree[i] != -1) {
			map<int, vector<int>>::iterator candidateListIterator = hyperGraph->getLabelVertexList()->find(queryGraph->getVertexByVertexId(topThree[i]).label);
			if (candidateListIterator == hyperGraph->getLabelVertexList()->end()) {
				// the query vertex doesn't have candidates
				return -1;
			}
			for (vector<int>::iterator candidateIterator = candidateListIterator->second.begin();
			candidateIterator != candidateListIterator->second.end(); candidateIterator++) {
				if (AdjacenceListsGRAPH_BOOST::degreeFilter(queryGraph, hyperGraph, topThree[i], *candidateIterator)) {
					topThreeNumberOfCandidates[i] ++;
				}
			}
			if (miniTopThree > topThreeNumberOfCandidates[i]) {
				us = topThree[i];
				miniTopThree = topThreeNumberOfCandidates[i];
			}
		}
	}
	return us;
}


void TurboIsoBoosted::rewriteToNecTree() {
	bool *flags = new bool[queryGraph->getNumberOfVertexes()];
	for (int i = 0; i<queryGraph->getNumberOfVertexes(); i++) {
		flags[i] = false;
	}

	queue<std::pair<int, int>> S; // id, parentId
	int vertexId, parentId;

	S.push(std::pair<int, int>(startQueryVertex, -1));
	flags[startQueryVertex] = true;

	while (!S.empty()) {
		vertexId = S.front().first;
		parentId = S.front().second;
		S.pop();
		necTree.push_back(NECNode());
		NECNode & necNode = *(necTree.rbegin());
		necNode.vertexId = vertexId;
		necNode.parent = parentId;
		necNode.id = necTree.size() - 1;
		necNode.label = queryGraph->getVertexByVertexId(vertexId).label;

		if (parentId != -1)
			necTree[parentId].childList.push_back(necNode.id);

		AdjacenceListsGRAPH::adjIterator vertexIterator(queryGraph, vertexId);
		for (AdjacenceListsGRAPH::link t = vertexIterator.begin(); !vertexIterator.end(); t = vertexIterator.next()) {
			if (!flags[t->v]) {
				flags[t->v] = true;
				S.push(std::pair<int, int>(t->v, necNode.id));
			}
		}
	}
	delete[] flags;
}

void TurboIsoBoosted::generateEquivalentEmbedding() {
	/*
	* becasue the starting query vertex also has considered the DRT infor. so we need to add up the qd-equivalent mappings for the starting vertex
	*/
	int startQueryVertexMappingId = embedding[startQueryVertex];
	AdjacenceListsGRAPH::Vertex * startQueryVertexMappingVertex = hyperGraph->getVertexAddressByVertexId(startQueryVertexMappingId);

	generateQDEquivalentEmbedding();

	for (std::vector<int>::iterator qdChildIterator = startQueryVertexMappingVertex->qdEquivalentVertexList.begin(); qdChildIterator != startQueryVertexMappingVertex->qdEquivalentVertexList.end();
	qdChildIterator++) {
		embedding[startQueryVertex] = *qdChildIterator;

		generateEquivalentEmbedding();

		//Generated equivalent embedddings
		//cout<<"Equivalent Embedding"<<endl; //@debug
	}

	// recover
	if (embedding[startQueryVertex] != startQueryVertexMappingId) {
		embedding[startQueryVertex] = startQueryVertexMappingId;
	}
}

void TurboIsoBoosted::generateQDEquivalentEmbedding() {

	//showEmbedding();
	int newFinalEmbeddings = 1;
	/*
	* the key represents the mapped hyperVertexId
	* the first int in the pair is the number of data vertices that the hyper vertex has
	* the second int in the pair indicate how many times(how many query vertex) this hypervertex is matched.
	*/
	map<int, std::pair<int, int>> distinctHyperVertexId;

	for (int pEindex = 0; pEindex < mappedQueryVertexSize; pEindex++) {
		map<int, std::pair<int, int>>::iterator mapTwoIntEndMapIterator = distinctHyperVertexId.find(embedding[pEindex]);
		if (mapTwoIntEndMapIterator == distinctHyperVertexId.end()) {
			distinctHyperVertexId.insert(
				std::pair<int, std::pair<int, int>>(
					embedding[pEindex],
					std::pair<int, int>(hyperGraph->getVertexByVertexId(embedding[pEindex]).sEquivalentVertexList.size(), 1)
					)
				);
		}
		else {
			mapTwoIntEndMapIterator->second.second += 1;
		}
	}

	for (map<int, std::pair<int, int>>::iterator mapTwoIntEndMapIterator = distinctHyperVertexId.begin(); mapTwoIntEndMapIterator != distinctHyperVertexId.end(); mapTwoIntEndMapIterator++) {
		if (mapTwoIntEndMapIterator->second.second > 1) {
			newFinalEmbeddings *= Math_Utility::combinations(mapTwoIntEndMapIterator->second.first, mapTwoIntEndMapIterator->second.second);
		}
		else {
			newFinalEmbeddings *= mapTwoIntEndMapIterator->second.first;
		}
	}

	//@MQO
	(*numberOfEmbeddings)[queryGraph->graphId] += newFinalEmbeddings;

}




void TurboIsoBoosted::showEmbedding() {
	std::cout << "{";
	for (int i = 0; i < queryGraph->getNumberOfVertexes(); i++) {
		cout << "*: " << i << "->" << embedding[i] << " , ";
	}
	cout << "}" << endl;
}
