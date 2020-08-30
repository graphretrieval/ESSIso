#include"AdjacenceListsGRAPH_BOOST.h"
#include<algorithm>
#include<fstream>
#include<set>
#include<vector>


using namespace std;



int isEquivalent(vector<int> & firstVertexList, vector<int> & secondVertexList, AdjacenceListsGRAPH::Vertex * vVertex, AdjacenceListsGRAPH::Vertex * uVertex);

int isContainment(vector<int> & bigVertexList, vector<int> & smallVertexList, AdjacenceListsGRAPH::Vertex * v, AdjacenceListsGRAPH::Vertex * u);


/**
* this function is based on that the adjacentce vertices have already been ordered
* this function is used after the DLF filter has been applied over the v and w vertices
* @return
*	0. is not a equivalent 
*   1. is query dependent equivalent and a clique
*   2. is query dependent equivalent but not clique
*/
int AdjacenceListsGRAPH_BOOST::isEquivalentRelation(AdjacenceListsGRAPH* dataGraph, AdjacenceListsGRAPH* queryGraph, int v, int w, int queryVertexId) { 

	AdjacenceListsGRAPH::Vertex* vVertex = dataGraph->getVertexAddressByVertexId(v);
	AdjacenceListsGRAPH::Vertex* wVertex = dataGraph->getVertexAddressByVertexId(w);
	int relationFlag = 0;

	if(vVertex->isClique + wVertex->isClique == 3){
		// they cannot be equivalent
		return 0;
	}
	if(dataGraph->edge(v, w)){
		relationFlag = 1;

		if(vVertex->isClique + wVertex->isClique == 2){
			return 0;
		}
	}else{
		if(vVertex->isClique + wVertex->isClique == 1){
			return 0;
		}
	}

	map<int, vector<int>>::iterator vVertexNeighbourIterator;
	map<int, vector<int>>::iterator wVertexNeighbourIterator;
	
	std::set<int>* labelSet = &(queryGraph->getVertexAddressByVertexId(queryVertexId)->labelSet);	

	for(std::set<int>::iterator queryLabelIterator = labelSet->begin(); queryLabelIterator!= labelSet->end(); queryLabelIterator++) {
		
		vVertexNeighbourIterator = vVertex->labelVertexList.find(*queryLabelIterator);
		wVertexNeighbourIterator = wVertex->labelVertexList.find(*queryLabelIterator);

		if((vVertexNeighbourIterator == vVertex->labelVertexList.end()) && (wVertexNeighbourIterator == wVertex->labelVertexList.end())){
			continue;
		}else if((vVertexNeighbourIterator == vVertex->labelVertexList.end()) ^ (wVertexNeighbourIterator == wVertex->labelVertexList.end())){
			return 0;
		}

		if(isEquivalent(vVertexNeighbourIterator->second, wVertexNeighbourIterator->second, vVertex, wVertex) == 0){
			return 0;
		}
	}
	
	if(relationFlag == 1){
		return 1;
	}else{
		return 2;
	}

}

/**
* this function is based on that the adjacentce vertices have already been ordered
*
* @return
*	0. is not a relation 
*	1. v query dependent contains w
*   2. w query dependent contains v
*   3. v query dependent equivalent to w
*/
int AdjacenceListsGRAPH_BOOST::isContainmentRelation(AdjacenceListsGRAPH* dataGraph, AdjacenceListsGRAPH* queryGraph, int v, int w, int queryVertexId){ 

	AdjacenceListsGRAPH::Vertex* vVertex = dataGraph->getVertexAddressByVertexId(v);
	AdjacenceListsGRAPH::Vertex* wVertex = dataGraph->getVertexAddressByVertexId(w);

	map<int, vector<int>>::iterator vVertexNeighbourIterator;
	map<int, vector<int>>::iterator wVertexNeighbourIterator;
	
	std::set<int>* labelSet = &(queryGraph->getVertexAddressByVertexId(queryVertexId)->labelSet);	
	int relationFlag = 0;
	
	for(std::set<int>::iterator queryLabelIterator = labelSet->begin(); queryLabelIterator!= labelSet->end(); queryLabelIterator++) {
		
		vVertexNeighbourIterator = vVertex->labelVertexList.find(*queryLabelIterator);
		wVertexNeighbourIterator = wVertex->labelVertexList.find(*queryLabelIterator);

		if((vVertexNeighbourIterator == vVertex->labelVertexList.end()) && (wVertexNeighbourIterator == wVertex->labelVertexList.end())){
			continue;
		}else if((vVertexNeighbourIterator == vVertex->labelVertexList.end()) ^ (wVertexNeighbourIterator == wVertex->labelVertexList.end())){
			return 0;
		}

		if(vVertexNeighbourIterator->second.size() > wVertexNeighbourIterator->second.size()){
			if(relationFlag == 2){
				return 0;
			}
			if(isContainment(vVertexNeighbourIterator->second, wVertexNeighbourIterator->second, vVertex, wVertex) == 0){
				return 0;
			}else{
				relationFlag = 1;
			}
		}else if(vVertexNeighbourIterator->second.size() < wVertexNeighbourIterator->second.size()){
			if(relationFlag == 1){
				return 0;
			}
			if(isContainment(wVertexNeighbourIterator->second, vVertexNeighbourIterator->second, wVertex, vVertex) == 0){
				return 0;
			}else{
				relationFlag = 2;
			}
		}else{
			if(isContainment(wVertexNeighbourIterator->second, vVertexNeighbourIterator->second, wVertex, vVertex) == 0){
				return 0;
			}else{
				if(relationFlag == 0){
					relationFlag = 3;
				}
			}
		}
	}

	return relationFlag;

}

/**
* 0 means they are not equivalent 
* 1 means they are equivalent 
*/
int isEquivalent(vector<int> & firstVertexList, vector<int> & secondVertexList, AdjacenceListsGRAPH::Vertex * vVertex, AdjacenceListsGRAPH::Vertex * uVertex){
	for(size_t i=0,j=0;i < firstVertexList.size() && j < secondVertexList.size(); ) {
		if(firstVertexList[i] == uVertex->id){
			i++;
		}
		else if(secondVertexList[j] == vVertex->id) {
			j++;
		}
		else if(firstVertexList[i] != uVertex->id && secondVertexList[j] != vVertex->id){
			if(firstVertexList[i] != secondVertexList[j]){
				return 0;
			}
			i++;
			j++;
		}
	}
	return 1;
}


/**
* 0 means first does not contains the second
* 1 means first does contains the second
*/
int isContainment(vector<int> & bigVertexList, vector<int> & smallVertexList, AdjacenceListsGRAPH::Vertex * vVertex, AdjacenceListsGRAPH::Vertex * uVertex){
	vector<int>::iterator vVertexNeighbourIterator = bigVertexList.begin();
	for(vector<int>::iterator uVertexNeighbourIterator = smallVertexList.begin(); uVertexNeighbourIterator != smallVertexList.end(); uVertexNeighbourIterator++){
		if(*uVertexNeighbourIterator == vVertex->id){
				continue;
		}
		while(*vVertexNeighbourIterator != *uVertexNeighbourIterator){
			vVertexNeighbourIterator ++;
			if(vVertexNeighbourIterator == bigVertexList.end()){
				return 0;
			}
		}
	}
	return 1;
}


/*
 * The degree filter for the hypergraph is more complicated than  the original subgraph isomorphism
 * The degree filter is to make sure, for each label of each query's neighbour label set, the candidate has more neighbours in this label
 * For a query vertex, if has a neighbour with the same label. However, a hypervertex may not has this label in its neighbour. But it contains a sequivalent vertice with this label in
 */
bool AdjacenceListsGRAPH_BOOST::degreeFilter(AdjacenceListsGRAPH* queryGraph, AdjacenceListsGRAPH* dataGraph, int u, int v){

	/*
	 * query vertex u and data vertex v share the same label
	 */
	AdjacenceListsGRAPH::Vertex * queryVertexU = queryGraph -> getVertexAddressByVertexId(u);
	AdjacenceListsGRAPH::Vertex * dataVertexV = dataGraph -> getVertexAddressByVertexId(v);
	
	std::set<int>* queryLabelSet = &queryVertexU->labelSet;


	for(std::set<int>::iterator setIterator = queryLabelSet->begin(); setIterator!= queryLabelSet->end(); setIterator++) {

		std::map<int,std::vector<int>>::iterator dataLabelVertexIterator = dataVertexV->labelVertexList.find(*setIterator);

		if(*setIterator == dataVertexV->label){
			size_t count = 0;
			count += dataVertexV->sEquivalentVertexList.size() - 1;
			if(dataLabelVertexIterator ==  dataVertexV->labelVertexList.end()){
				if(count < queryVertexU->labelVertexList.find(*setIterator)->second.size())
					return false;
			}else {
				for(vector<int>::iterator neighbourIterator = dataLabelVertexIterator->second.begin(); neighbourIterator != dataLabelVertexIterator->second.end(); neighbourIterator++){
					count += dataGraph->getVertexAddressByVertexId(*neighbourIterator)->sEquivalentVertexList.size();
				}
				if(count < queryVertexU->labelVertexList.find(*setIterator)->second.size())
					return false;	
			}
		}else{
			if(dataLabelVertexIterator ==  dataVertexV->labelVertexList.end()){
				return false;
			}else{
				int count = 0;
				for(vector<int>::iterator neighbourIterator = dataLabelVertexIterator->second.begin(); neighbourIterator != dataLabelVertexIterator->second.end(); neighbourIterator++){
					count += dataGraph->getVertexAddressByVertexId(*neighbourIterator)->sEquivalentVertexList.size();
				}
				if(count < queryVertexU->labelVertexList.find(*setIterator)->second.size())
					return false;
			}	
		}
	}

	return true;
	
}


/*
* For turboIso, we only build the DRT table for the starting query vertex.
* the d_table contains all the DRT units built over the candidates of this query vertex
* the superStartingandidates are the DRT units in the d_table which are not qdc-contained by any other DRT unit.
*/
void AdjacenceListsGRAPH_BOOST::computeDTable(AdjacenceListsGRAPH* queryGraph, int queryVertex, AdjacenceListsGRAPH* dataGraph, std::map<int, vector<int>* >& candidates, 
	std::queue<int> & startingQueryVertexCandidates) {

		for(vector<int>::iterator candidateIterator = candidates[queryVertex]->begin(); candidateIterator != candidates[queryVertex]->end(); candidateIterator++) {
			AdjacenceListsGRAPH::Vertex * vertex = dataGraph->getVertexAddressByVertexId(*candidateIterator);
			vertex->qdeEquivalentHidden = false;
			vertex->qdContainedVertexList = std::vector<int>();
			vertex->qdIndegree = 0;
			vertex->qdSpongeIndegree = 0;
			vertex->qdEquivalentVertexList = std::vector<int>();
		}

		for(vector<int>::iterator candidateIterator = candidates[queryVertex]->begin(); candidateIterator != candidates[queryVertex]->end(); candidateIterator++) {

			AdjacenceListsGRAPH::Vertex * vertex = dataGraph->getVertexAddressByVertexId(*candidateIterator);

			if(vertex->qdeEquivalentHidden == true) {
				continue;
			}else{
				//currently we incorporate a degree filter at here
				if(!AdjacenceListsGRAPH_BOOST::degreeFilter(queryGraph, dataGraph, queryVertex, *candidateIterator)){
					continue;
				}
			}

			/*
			* start checking its following candidates to get all its equivalent vertices
			*/
			vector<int>::iterator innerLooper = candidateIterator + 1;
			while(innerLooper != candidates[queryVertex]->end()) {
				AdjacenceListsGRAPH::Vertex * innerVertex = dataGraph->getVertexAddressByVertexId(*innerLooper);
				if(innerVertex->qdeEquivalentHidden == true){
					innerLooper ++;
					continue;
				}
				int relationFlag = AdjacenceListsGRAPH_BOOST::isContainmentRelation(dataGraph, queryGraph, *candidateIterator, *innerLooper, queryVertex);
				if(relationFlag == 0){
					innerLooper ++;
					continue;	
				}

				if(relationFlag == 3) {
					
					vertex->qdEquivalentVertexList.push_back(*innerLooper);
					innerVertex->qdeEquivalentHidden = true;

				}else if(relationFlag == 1){
					
					innerVertex->qdIndegree ++;
					innerVertex->qdSpongeIndegree++;

					vertex->qdContainedVertexList.push_back(*innerLooper);

				}else if(relationFlag == 2){
					
					vertex->qdIndegree ++;
					vertex->qdSpongeIndegree ++;

					innerVertex->qdContainedVertexList.push_back(*innerLooper);
				}

				innerLooper ++;
			}

			
			/*
			 * After computing the qd-relationship. get the final vertices not qd or s contained by any others. 
			 */
			if(vertex->qdIndegree == 0){
				startingQueryVertexCandidates.push(*candidateIterator);
			}
		}

}



/*
 * For each label of the graph, we attach a list of s-root vertices to it, these vertices have no s-contained parents.
 */
void AdjacenceListsGRAPH_BOOST::buildLabelRootMap(AdjacenceListsGRAPH & graph){

	std::map<int,std::vector<int>> * labelVertexList = graph.getLabelVertexList();
	std::map<int,std::vector<int>> * labelContainmentRootList = graph.getLabelContainmentRootList();

	for(std::map<int,std::vector<int>>::iterator labelVertexIterator = labelVertexList->begin(); labelVertexIterator != labelVertexList -> end(); labelVertexIterator++){

		labelContainmentRootList->insert(std::pair<int, std::vector<int>>(labelVertexIterator->first,std::vector<int>()));
		std::map<int,std::vector<int>>::iterator newLabelContainmentRoots = labelContainmentRootList->find(labelVertexIterator->first);

		for(std::vector<int>::iterator vertexIterator = labelVertexIterator->second.begin(); vertexIterator != labelVertexIterator->second.end(); vertexIterator ++){
			if(graph.getVertexAddressByVertexId(*vertexIterator)->sIndegree != 0){
				continue;
			}
			newLabelContainmentRoots->second.push_back(*vertexIterator);
		}

	}
}

/*
 * The containment graph shares the same set of vertex set with the hypergraph. Therefore, we can integrate the data structure of containment relationships
 * into the hypergraph
 */
void AdjacenceListsGRAPH_BOOST::loadContainmentGraph(vector<AdjacenceListsGRAPH> & hyperGraphVector, std::ifstream & containmentGraphFile){

	string line;
	vector<int> integerValues;
	int graphIndex = 0;
	getline (containmentGraphFile,line);
	if(line.size() != 0 && (*line.begin()) == 't') {
		// A new graph
		AdjacenceListsGRAPH & graph = hyperGraphVector[graphIndex];
		std::vector<AdjacenceListsGRAPH::Vertex> * vertexList = graph.getVertexList();

		// insert all the edges
		while(*line.begin() == 'e') {

			String_Utility::readIntegersFromString(line, integerValues);
			(*vertexList)[*integerValues.begin()].sContainedVertexList.push_back(*(integerValues.begin()+1));
			(*vertexList)[*(integerValues.begin()+1)].sIndegree ++;
			(*vertexList)[*(integerValues.begin()+1)].spongeIndegree ++;

			getline(containmentGraphFile, line);
		}
	}

}