#include"QueryExecutionOrder.h"

using namespace std;


QueryExecutionOrder::QueryExecutionOrder(std::vector<PCM_Node> * pPatternContainmentMap, std::vector<ExecutionOrderNode> * pExecutionOrder) {
	patternContainmentMap = pPatternContainmentMap;
	executionOrder = pExecutionOrder;
}

QueryExecutionOrder::~QueryExecutionOrder() {
	
}

void QueryExecutionOrder::computeExecutionOrder() {

	vector<int> roots;
	int nextGraphId;
	for (std::vector<PCM_Node>::iterator iterator = patternContainmentMap -> begin(); iterator != patternContainmentMap -> end(); iterator++) {
		/*
		* As the PCM contains all the graphs no matter whether the query is isomorphic to another or not.
		* the isHidden is a flag to indicate the isomorphic relationship.
		*/
		if (iterator->isHidden) {
			continue;
		}
		if (iterator->parent.size() == 0) {
			roots.push_back(iterator->graphId);
		}
	}
	while ((nextGraphId = getNextPcmNodeId(roots)) != -1) {
		DFSTopological(nextGraphId);
	}
}

int QueryExecutionOrder::getNextPcmNodeId(vector<int> & candidates) {
	int index = -1;
	int maxWeight = -1;
	/*
	* given a list of candidates, get the one with the highest score.
	*/
	for (unsigned int i = 0; i<candidates.size(); i++) {
		if (!(*patternContainmentMap)[candidates[i]].isVisited && (*patternContainmentMap)[candidates[i]].numberOfComputedParents == (*patternContainmentMap)[candidates[i]].parent.size()) {
			if ((*patternContainmentMap)[candidates[i]].sameHeightPriority > maxWeight) {
				index = i;
				maxWeight = (*patternContainmentMap)[candidates[i]].sameHeightPriority;
			}
		}
	}
	if (index == -1) {
		return -1;
	}
	else {
		return candidates[index];
	}
}




void QueryExecutionOrder::DFSTopological(int pcmId) {

	ExecutionOrderNode executionOrderNode;

	executionOrderNode.pcmId = pcmId;

	/*
	* After this query, increase computed children of its parents
	*/
	for (vector<int>::iterator parentIterator = (*patternContainmentMap)[pcmId].parent.begin(); parentIterator != (*patternContainmentMap)[pcmId].parent.end(); parentIterator++) {
		(*patternContainmentMap)[*parentIterator].numberOfComputedChildren++;
		if ((*patternContainmentMap)[*parentIterator].numberOfComputedChildren == (*patternContainmentMap)[*parentIterator].children.size()) {
			// release the memory of its parents
			executionOrderNode.releaseParentsPcmId.push_back(*parentIterator);
		} 
	}

	(*patternContainmentMap)[pcmId].isVisited = true;
	executionOrder->push_back(executionOrderNode);


	/*
	* for each of its children, we increase its number of computed parents.
	*/
	if ((*patternContainmentMap)[pcmId].children.size() > 0) {
		for (vector<int>::iterator childIterator = (*patternContainmentMap)[pcmId].children.begin(); childIterator != (*patternContainmentMap)[pcmId].children.end(); childIterator++) {
			if (!(*patternContainmentMap)[*childIterator].isVisited) {
				(*patternContainmentMap)[*childIterator].numberOfComputedParents++;
				if ((*patternContainmentMap)[*childIterator].numberOfComputedParents != (*patternContainmentMap)[*childIterator].parent.size()) {
					/*
					* Not all of its parents have been computed. It is meaningless still to update if all its parents have already been computed.
					*/
					changeParentsWeight(*childIterator);
				}
			}
		}

		int nextPcmNodeId = getNextPcmNodeId((*patternContainmentMap)[pcmId].children);
		while (nextPcmNodeId != -1) {
			DFSTopological(nextPcmNodeId);
			nextPcmNodeId = getNextPcmNodeId((*patternContainmentMap)[pcmId].children);
		}
	}

}




void QueryExecutionOrder::changeParentsWeight(int graphId) {
	for (vector<int>::iterator parentIterator = (*patternContainmentMap)[graphId].parent.begin(); parentIterator != (*patternContainmentMap)[graphId].parent.end(); parentIterator++) {
		if (!(*patternContainmentMap)[*parentIterator].isVisited) {
			(*patternContainmentMap)[*parentIterator].sameHeightPriority++;
			changeParentsWeight(*parentIterator);
		}
	}
}