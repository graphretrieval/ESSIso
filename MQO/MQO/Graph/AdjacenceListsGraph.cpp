#include"AdjacenceListsGraph.h"
#include<iostream>
#include<set>
#include <algorithm>    // std::sort
#include"Utility.h"
#include<stack>
#include<queue>
#include<map>
#include"GlobalConstant.h"


// Overload 
bool operator<(const TLSequence& a, const TLSequence& b) {
	if(a.pivot < b.pivot){
		return true;
	}else if(a.pivot == b.pivot){
		if(a.start < b.start){
			return true;
		}else if(a.start == b.start){
			return a.end < b.end;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

AdjacenceListsGRAPH::~AdjacenceListsGRAPH() {
	clear();
}

AdjacenceListsGRAPH::AdjacenceListsGRAPH() {
	digraph = false;
	numberOfEdges = 0;
	numberOfVertex = 0;
	totalTLSNumber = 0;
	graphId = -1;
	nonadjList = std::vector<std::set<int>>();
	adjList = std::vector<std::set<int>>();
}


std::vector<AdjacenceListsGRAPH::Edge> *  AdjacenceListsGRAPH::getEdgeList(){
	return & edgeList;
}

std::vector<AdjacenceListsGRAPH::Vertex> * AdjacenceListsGRAPH::getVertexList(){
	return & vertexList;
}
std::map<std::pair<int, int>, std::vector<std::pair<int, int>>> * AdjacenceListsGRAPH::getVertexLabelsEdgeList(){
	return & vertexLabelsEdgeList;
}

void AdjacenceListsGRAPH::buildIndex() {
	index = new faiss::IndexFlatL2(GlobalConstant::embeddingDimension);
	index->add(getNumberOfVertexes(), getEmbeddingMatrix());
}

faiss::IndexFlatL2 * AdjacenceListsGRAPH::getIndex() {
	return index;
}

float* AdjacenceListsGRAPH::getEmbeddingMatrix() {
	return embeddingMatrix;
}
void AdjacenceListsGRAPH::buildEmbeddingMatrix(){
	embeddingMatrix = new float[numberOfVertex * GlobalConstant::embeddingDimension];
	for(int vertexId = 0; vertexId < numberOfVertex; vertexId++) {
		AdjacenceListsGRAPH::Vertex dataVertex = getVertexByVertexId(vertexId);
		for(int j = 0; j < GlobalConstant::embeddingDimension; j++){
			embeddingMatrix[GlobalConstant::embeddingDimension * vertexId + j] = dataVertex.embeddingVector.at(j);
		}           
	}
	// for(int i = 0; i <  numberOfVertex; i++) {
	// 	for(int j = 0; j < 3; j++)
	// 		printf("%f ",embeddingMatrix[i * 3 + j]);
	// 	printf("\n");
    // }

}
/**
* Each label following the vertices whose label is this label
*/
void AdjacenceListsGRAPH::buildLabelVertexList(){
	for(std::vector<AdjacenceListsGRAPH::Vertex>::iterator vertexIterator = vertexList.begin(); vertexIterator != vertexList.end(); vertexIterator++){
		std::map<int,std::vector<int>>::iterator labelVertexListIterator = labelVertexList.find(vertexIterator->label);
		if(labelVertexListIterator != labelVertexList.end()){
			// label already exists
			(labelVertexListIterator -> second).push_back(vertexIterator -> id);
		}
		else{
			// a new label
			labelVertexList.insert(std::pair<int, std::vector<int>>(vertexIterator->label,std::vector<int>()));
			labelVertexList.find(vertexIterator->label)->second.push_back(vertexIterator -> id);
		}
	}
}

void AdjacenceListsGRAPH::buildVertexLabelEdgeList(){

	/*
	 * Only save A->B where A is less than B, 
	 * also need to make sure the vertex from-to is corresponding to the label A -> B
	 * Notice that, A-B: (uA, uB). The id of uA is not necessary less than uB. Because the vertex of uA has label A, and uB has label B
	 */
	for(std::vector<AdjacenceListsGRAPH::Edge>::iterator edgeIterator = edgeList.begin(); edgeIterator != edgeList.end(); edgeIterator++){
		int sourceLabel = getVertexAddressByVertexId(edgeIterator->source)->label;
		int desiLabel = getVertexAddressByVertexId(edgeIterator->destination)->label;
	
		bool swaped = false;
		if(sourceLabel > desiLabel){
			swaped = true;
		}

		std::map<std::pair<int,int>,std::vector<std::pair<int, int>>>::iterator labelVertexListIterator;

		if(swaped){
			labelVertexListIterator = vertexLabelsEdgeList.find(std::pair<int,int>(desiLabel,sourceLabel));
		}else{
			labelVertexListIterator = vertexLabelsEdgeList.find(std::pair<int,int>(sourceLabel,desiLabel));
		}

		if(labelVertexListIterator != vertexLabelsEdgeList.end()){
			// label pair already exists
			if(swaped){
				(labelVertexListIterator -> second).push_back(std::pair<int, int>(edgeIterator->destination, edgeIterator->source));
			}else{
				(labelVertexListIterator -> second).push_back(std::pair<int, int>(edgeIterator->source, edgeIterator->destination));	
			}
		}
		else{
			// a new label pair
			std::vector<std::pair<int, int>> labelPair;
			if(swaped){
				labelPair.push_back(std::pair<int, int>(edgeIterator->destination, edgeIterator->source));
				vertexLabelsEdgeList.insert(std::pair<std::pair<int,int>,std::vector<std::pair<int, int>>>(std::pair<int,int>(desiLabel,sourceLabel), labelPair));
			}else{
				labelPair.push_back(std::pair<int, int>(edgeIterator->source, edgeIterator->destination));	
				vertexLabelsEdgeList.insert(std::pair<std::pair<int,int>,std::vector<std::pair<int, int>>>(std::pair<int,int>(sourceLabel,desiLabel), labelPair));
			}
		}
	}
}

/**
* For each vertex, each label of its neighbour, following the neighbour vertices whose label is this label
* u1:  A: (u2,u3)
*      B: (u4,u5)
*/
void AdjacenceListsGRAPH::buildVertexLabelVertexList(){

	std::map<int,std::vector<int>>::iterator vertexLabelVertexIterator;

	for(std::vector<AdjacenceListsGRAPH::Vertex>::iterator vertexIterator = vertexList.begin(); vertexIterator != vertexList.end(); vertexIterator++){
		
		AdjacenceListsGRAPH::node* adjVertex = vertexIterator->adj;
		while(adjVertex != 0){
			vertexLabelVertexIterator = vertexIterator->labelVertexList.find(vertexList[adjVertex->v].label);
			if(vertexLabelVertexIterator != vertexIterator->labelVertexList.end()){
				vertexLabelVertexIterator->second.push_back(adjVertex->v);
			}else{
				// a new label
				std::vector<int> newVertexLabelList;
				newVertexLabelList.push_back(adjVertex->v);
				vertexIterator->labelVertexList.insert(std::pair<int,std::vector<int>>(vertexList[adjVertex->v].label,newVertexLabelList));
				//insert into labelset
				vertexIterator->labelSet.insert(vertexList[adjVertex->v].label);
			}
			adjVertex = adjVertex->next;
		}
	}
}

void AdjacenceListsGRAPH::buildTLSequence(){

	AdjacenceListsGRAPH::Vertex * startVertex;
	AdjacenceListsGRAPH::Vertex * endVertex;

	for(std::vector<AdjacenceListsGRAPH::Vertex>::iterator vertexIterator = vertexList.begin(); vertexIterator != vertexList.end(); vertexIterator++){

		link adjacenceVertex = vertexIterator->adj;
		while(adjacenceVertex != 0){

			link innerAdjacenceVertex = adjacenceVertex->next;
			while(innerAdjacenceVertex != 0){

				if (vertexList[(*adjacenceVertex).v].label < vertexList[(*innerAdjacenceVertex).v].label) {
					startVertex = &vertexList[(*adjacenceVertex).v];
					endVertex = &vertexList[(*innerAdjacenceVertex).v];
				}
				else {
					startVertex = &vertexList[(*innerAdjacenceVertex).v];
					endVertex = &vertexList[(*adjacenceVertex).v];
				}

				TLSequence tcNode;
				tcNode.pivot = vertexIterator->label;
				tcNode.start = startVertex->label;
				tcNode.end = endVertex->label;
				
				std::map<TLSequence, std::vector<std::vector<int>>>::iterator tcNodeIterator = TLSequenceMap.find(tcNode);
				if (tcNodeIterator == TLSequenceMap.end()) {
					TLSequenceMap.insert(std::pair<TLSequence, std::vector<std::vector<int>>>(tcNode, std::vector<std::vector<int>>()));
					tcNodeIterator = TLSequenceMap.find(tcNode);
				}
				tcNodeIterator->second.push_back(std::vector<int>());

				std::vector<int> & verticeList = tcNodeIterator->second[tcNodeIterator->second.size() - 1];
				verticeList.push_back(startVertex->id);
				verticeList.push_back(vertexIterator->id);
				verticeList.push_back(endVertex->id);

				totalTLSNumber++;

				innerAdjacenceVertex = innerAdjacenceVertex->next;
			}

			adjacenceVertex = adjacenceVertex->next;
		}
	}
}

void AdjacenceListsGRAPH::buildDFSTraversalOrder(){
	if (DFSTraversalOrder.size() > 0) {
		return;
	}

	bool* flags = new bool[vertexList.size()]();
	for(size_t i=0; i<vertexList.size(); i++){
		flags[i] = false;	
	}

	std::stack<int> S; // id, parentId
	int id;
	// start from 0
	S.push(0);

	while(!S.empty()){

		id = S.top();
		S.pop();

		if(!flags[id]){
			flags[id] = true;
			DFSTraversalOrder.push_back(id);

			AdjacenceListsGRAPH::link neighbourLink = vertexList[id].adj;
			while(neighbourLink != NULL){
				if(!flags[neighbourLink->v]){
					S.push(neighbourLink->v);
				}
				neighbourLink = neighbourLink->next;
			}
		}

	}

	delete []flags;
}

void AdjacenceListsGRAPH::buildComboGraph(int maximumWidth){
	/*
	 * We modified the algorithm proposed in "Quotient Tree Partitioning of Undirected Graphs"
	 * We start with the maximum degree vertex as level 1
	 */
	std::map<int, std::vector<int>> levelVertexInverted;
	int * level = new int[vertexList.size()];
	int maximumDegree = vertexList[0].inDegree;
	int maximumVertex = 0;

	bool* BFSFlags = new bool[vertexList.size()]();

	for (size_t i = 0; i<vertexList.size(); i++) {
		BFSFlags[i] = false;
		level[i] = -1;
		if (vertexList[i].inDegree > maximumDegree) {
			maximumVertex = i;
			maximumDegree = vertexList[i].inDegree;
		}
	}

	std::queue<int> BFS_queue; // id, parentId
	int BFS_id;
	// start from the maximum degree vertex
	level[maximumVertex] = 1;

	std::vector<int> startLevelGroup;
	startLevelGroup.push_back(maximumVertex);
	levelVertexInverted.insert(std::pair<int, std::vector<int>>(1, startLevelGroup));
	
	BFSFlags[maximumVertex] = true;
	BFS_queue.push(maximumVertex);

	while (!BFS_queue.empty()) {

		BFS_id = BFS_queue.front();
		BFS_queue.pop();

		AdjacenceListsGRAPH::link neighbourLink = vertexList[BFS_id].adj;
		while (neighbourLink != NULL) {
			if (!BFSFlags[neighbourLink->v]) {
				
				BFS_queue.push(neighbourLink->v);
				BFSFlags[neighbourLink->v] = true;

				level[neighbourLink->v] = level[BFS_id] + 1;

				std::map<int, std::vector<int>>::iterator levelIterator = levelVertexInverted.find(level[neighbourLink->v]);
				if (levelIterator != levelVertexInverted.end()) {
					levelIterator->second.push_back(neighbourLink->v);
				}
				else {
					std::vector<int> newLevelGroup;
					newLevelGroup.push_back(neighbourLink->v);
					levelVertexInverted.insert(std::pair<int, std::vector<int>>(level[neighbourLink->v], newLevelGroup));
				}
			}
			neighbourLink = neighbourLink->next;
		}
	}
	delete [] BFSFlags;

	bool* groupedFlags = new bool[vertexList.size()]();
	for (size_t i = 0; i < vertexList.size(); i++) {
		groupedFlags[i] = false;
	}
	for (std::map<int, std::vector<int>>::reverse_iterator levelInverseIterator = levelVertexInverted.rbegin(); levelInverseIterator != levelVertexInverted.rend(); levelInverseIterator++) {
		for (size_t i = 0; i < levelInverseIterator->second.size(); i++) {
			if (groupedFlags[levelInverseIterator->second[i]]) {
				continue;
			}
			groupedFlags[levelInverseIterator->second[i]] = true;

			ComboNODE comboNode;
			comboNode.queryVertices.push_back(levelInverseIterator->second[i]);
			for (size_t j = i+1; j < levelInverseIterator->second.size();j++) {
				if (comboNode.queryVertices.size() == maximumWidth) {
					break;
				}
				if (!groupedFlags[levelInverseIterator->second[j]]) {
					if (edge(levelInverseIterator->second[i], levelInverseIterator->second[j])) {
						comboNode.queryVertices.push_back(levelInverseIterator->second[j]);
						groupedFlags[levelInverseIterator->second[j]] = true;
					}
					else {
						AdjacenceListsGRAPH::link neighbourLink = vertexList[i].adj;
						while (neighbourLink != NULL) {
							if (level[neighbourLink->v] == levelInverseIterator->first + 1) {
								if (edge(neighbourLink->v, levelInverseIterator->second[j])) {
									
									comboNode.queryVertices.push_back(levelInverseIterator->second[j]);
									groupedFlags[levelInverseIterator->second[j]] = true;

									break;
								}
							}
							neighbourLink = neighbourLink->next;
						}
					}
				}
			}

			comboGraph.push_back(comboNode);
		}
	}

	comboGraphEdgeMatrix = new short int*[comboGraph.size()];
	for(unsigned int i=0; i<comboGraph.size(); i++){
		comboGraphEdgeMatrix[i] = new short int[comboGraph.size()];
		for(unsigned int j=0; j<comboGraph.size();j++){
			comboGraphEdgeMatrix[i][j] = 0;
		}
	}
	
	// use a matrix to save the combo edges
	for(size_t i=0; i<comboGraph.size(); i++){
		for(size_t j=i+1; j<comboGraph.size(); j++){
			for(size_t k = 0; k< comboGraph[i].queryVertices.size(); k++){
				for(size_t l=0; l< comboGraph[j].queryVertices.size();l++){
					if(edge(comboGraph[i].queryVertices[k], comboGraph[j].queryVertices[l])){
						comboGraph[i].neighbours.insert(j);
						comboGraph[j].neighbours.insert(i);
						comboGraphEdgeMatrix[i][j] = 1;
						comboGraphEdgeMatrix[j][i] = 1;
						break;
					}
				}
			}
		}
	}

	// DFS visiting order for the combo graph
	bool* flags = new bool[comboGraph.size()]();
	for(size_t i=0; i<comboGraph.size(); i++){
		flags[i] = false;	
	}

	std::stack<std::pair<int,int>> S; // id, parentId
	int id, parentId;
	// start from 0
	S.push(std::pair<int,int>(0,-1));

	while(!S.empty()){
		id = S.top().first;
		parentId = S.top().second;

		S.pop();
		if(!flags[id]){
			flags[id] = true;
			DFSComboVertexOrder.push_back(id);

			comboGraph[id].parent = parentId;
			if(parentId != -1){
				comboGraph[parentId].children.push_back(id);
			}

			for(std::set<int>::iterator neighbourIterator = comboGraph[id].neighbours.begin(); neighbourIterator != comboGraph[id].neighbours.end(); neighbourIterator++){
				if(!flags[*neighbourIterator]){
					S.push(std::pair<int,int>(*neighbourIterator,id));
				}
			}
		}

	}

	delete []flags;

}

void AdjacenceListsGRAPH::cleanComboInfo() {

	comboGraph = std::vector<ComboNODE>();
	delete comboGraphEdgeMatrix;
	DFSComboVertexOrder = std::vector<int>();

}

std::map<int,std::vector<int>> * AdjacenceListsGRAPH::getLabelVertexList(){
	return & labelVertexList;
}

bool AdjacenceListsGRAPH::directed() const{
	return digraph;
}

void AdjacenceListsGRAPH::insert(Vertex v){
	vertexList.push_back(v);
	labelSet.insert(v.label); // insert the label into the labelset
	numberOfVertex ++;
}

int AdjacenceListsGRAPH::degree(int v) {
	if(!digraph){
		return vertexList[v].inDegree; // indegree is the same as outdegree
	}else{
		return vertexList[v].inDegree + vertexList[v].outDegree;
	}
}

void AdjacenceListsGRAPH::initAdjSet() {
	adjList = {vertexList.size(), std::set<int>()};
	nonadjList = std::vector<std::set<int>>();
	std::set<int> tempSet;
	for (int i =0 ;i <vertexList.size(); i++)
		tempSet.insert(i);
	for (int i =0 ;i <vertexList.size(); i++){
		nonadjList.push_back(tempSet);
		nonadjList[i].erase(i);
	}
}

/*
 * // random order
 */
void AdjacenceListsGRAPH::insertRandom(Edge e){
	int v = e.source, w = e.destination, l = e.label;
	vertexList[v].adj = new node(w, vertexList[v].adj, e.label);
	vertexList[v].outDegree++;
	vertexList[w].inDegree++;
	if(!digraph) {
		vertexList[w].adj = new node(v, vertexList[w].adj, e.label);
		vertexList[w].outDegree++;
		vertexList[v].inDegree++;
	}
	numberOfEdges ++;

	edgeList.push_back(e);
}

//For the performance, we put the adjacence list in an ascending order
void AdjacenceListsGRAPH::insert(Edge e, bool isDataGraph) {
	int v = e.source, w = e.destination, l = e.label;
	if (!isDataGraph) {
		adjList[v].insert(w);
		nonadjList[v].erase(w);
	}
	if(vertexList[v].adj == 0){
		vertexList[v].adj = new AdjacenceListsGRAPH::node(w, 0, e.label);
	}
	else {
		insert_order_helper(&vertexList[v], w, e.label);
	}

	vertexList[v].outDegree++;
	vertexList[w].inDegree++;
	if(!digraph) {
		if (!isDataGraph) {
			adjList[w].insert(v);
			nonadjList[w].erase(v);
		}
		if(vertexList[w].adj == 0) {
			vertexList[w].adj = new AdjacenceListsGRAPH::node(v, 0, e.label);
		}
		else {
			insert_order_helper(&vertexList[w], v, e.label);
		}
		vertexList[w].outDegree++;
		vertexList[v].inDegree++;
	}
	numberOfEdges ++;

	edgeList.push_back(e);
}

void AdjacenceListsGRAPH::insert_order_helper(AdjacenceListsGRAPH::Vertex* vertex, int  w, int edgeLabel){

	AdjacenceListsGRAPH::link looper = vertex->adj;
	AdjacenceListsGRAPH::link lastScannedNode = vertex->adj;
	
	if(looper->v > w){
		vertex->adj = new AdjacenceListsGRAPH::node(w, vertex->adj, edgeLabel);
		return ;
	}else{
		looper = looper->next;
	}

	while(looper != 0 && looper->v < w){
		looper = looper->next;
		lastScannedNode = lastScannedNode -> next;
	}
	
	lastScannedNode->next = new AdjacenceListsGRAPH::node(w, lastScannedNode->next, edgeLabel);

}



int AdjacenceListsGRAPH::getNumberOfVertexes() const {
	return numberOfVertex;
}
int AdjacenceListsGRAPH::getNumberOfEdges() const {
	return numberOfEdges;
}

void AdjacenceListsGRAPH::clear(){
	numberOfVertex = 0;
	numberOfEdges = 0;
	digraph = false;
	graphId = -1;

	vertexList = std::vector<AdjacenceListsGRAPH::Vertex>();
	edgeList = std::vector<Edge>();
	DFSTraversalOrder = std::vector<int>();

	labelSet = std::set<int>();
	labelVertexList = std::map<int, std::vector<int>>();
	vertexLabelsEdgeList = std::map<std::pair<int, int>, std::vector<std::pair<int, int>>>();

	TLSequenceMap = std::map<TLSequence, std::vector<std::vector<int>>>();
	totalTLSNumber = 0;

	comboGraph = std::vector<ComboNODE>();
	DFSComboVertexOrder = std::vector<int>();

	labelContainmentRootList = std::map<int, std::vector<int>>();
}

bool AdjacenceListsGRAPH::edge(int v, int w){
	link adjacenceNode = vertexList[v].adj;
	while(adjacenceNode != 0){
		if((*adjacenceNode).v == w)
			return true;
		adjacenceNode = adjacenceNode->next;
	}
	return false;
}


AdjacenceListsGRAPH::adjIterator::adjIterator(const AdjacenceListsGRAPH* GRef, int v):G(GRef),v(v){
	t=0;
}
AdjacenceListsGRAPH::link AdjacenceListsGRAPH::adjIterator::begin(){
	t = G->vertexList[v].adj;
	return t?t:0;
}
AdjacenceListsGRAPH::link AdjacenceListsGRAPH::adjIterator::next(){
	if(t) t = t->next; return t?t:0;
}
bool AdjacenceListsGRAPH::adjIterator::end() {
	if(t==0) return true;
	else return false;
}

AdjacenceListsGRAPH::Vertex AdjacenceListsGRAPH::getVertexByVertexId(int vertexId){
	return vertexList[vertexId];
}


AdjacenceListsGRAPH::Vertex* AdjacenceListsGRAPH::getVertexAddressByVertexId(int vertexId){
	return &vertexList[vertexId];
}

std::set<int>* AdjacenceListsGRAPH::getLabelSet(){
	return &labelSet;
}

std::map<TLSequence, std::vector<std::vector<int>>> * AdjacenceListsGRAPH::getTLSequenceMap(){
	return &TLSequenceMap;
}

int AdjacenceListsGRAPH::getTotalTLSNumber()
{
	return totalTLSNumber;
}

std::vector<ComboNODE> *  AdjacenceListsGRAPH::getComboGraph(){
	return &comboGraph;
}

short int ** AdjacenceListsGRAPH::getComboGraphEdgeMatrix(){
	return comboGraphEdgeMatrix;
}

std::vector<int> * AdjacenceListsGRAPH::getDFSComboVertexOrder(){
	return &DFSComboVertexOrder;
}

std::vector<int> * AdjacenceListsGRAPH::getDFSTraversalOrder(){
	return &DFSTraversalOrder;
}

std::map<int, std::vector<int>> * AdjacenceListsGRAPH::getLabelContainmentRootList() {
	return &labelContainmentRootList;
}

void AdjacenceListsGRAPH::printLabelEdges(std::ofstream * resultFile) {
	for (std::map<std::pair<int, int>, std::vector<std::pair<int, int>>>::iterator labelPairEdgeIter = vertexLabelsEdgeList.begin(); labelPairEdgeIter != vertexLabelsEdgeList.end(); labelPairEdgeIter++) {
		(*resultFile) << "(" << labelPairEdgeIter->first.first << "," << labelPairEdgeIter->first.second << ") : " << labelPairEdgeIter->second.size() << std::endl;
	}
}