#include <stdlib.h>
#include <stdio.h>
#include "FindAllMaximalCliques.h"
#include"PCMBuilder.h"
#include"GlobalConstant.h"


// The algorithm for finding all the maximal cliques
// This algorithm is writtedn using C for efficience considerations
// An implementation of Bron-Kerbosch algorithm
// From Algorithm 457 of the Collected Algorithms from CACM
// http://www.netlib.org/tomspdf/457.pdf
// According to the paper: "Finding All Cliques of an Undirected Graph"

FindAllMaximalCliques::FindAllMaximalCliques(){}

FindAllMaximalCliques::FindAllMaximalCliques(std::vector<AdjacenceListsGRAPH> * pQueryGraphVector) {
	queryGraphVector = pQueryGraphVector;
	N = queryGraphVector->size();
}

FindAllMaximalCliques::~FindAllMaximalCliques(){}



void FindAllMaximalCliques::findMaxclique(const bool ** pGraph, std::vector<std::vector<int>> * pSimiliarQueryGroups, std::vector<PCM_Node> * pPatternContainmentMap) {
	
	/*
	* Make sure the Diagonal are all "1", otherwise you will encounter "s" has been used without initized
	*/
	graph = pGraph;
	compsub.nodeInit(N);
	cliques = pSimiliarQueryGroups;
	patternContainmentMap = pPatternContainmentMap;
	/*
	 * Start finding maximal cliques within give graph
	 */
	int i;
	int *all = (int *)malloc(N*sizeof(int));
	for (i = 0; i<N; i++)
		all[i] = i;
	bkv2(all, 0, N);

	free(all);
}

// recursive function version 2 of Bron-Kerbosch algorithm
void FindAllMaximalCliques::bkv2(int* pOld, int ne, int ce) {

	int *newSet = (int *)malloc(ce*sizeof(int));
	int nod, fixp;
	int newne, newce, i, j, count, pos, p, s, sel, minnod;

	minnod = ce;
	nod = 0;
	// Determine each counter value and look for minimum
	for (i = 0; i <ce && minnod != 0; i++) {
		p = pOld[i];
		count = 0;
		pos = 0;
		// Count disconnections
		for (j = ne; j < ce && count < minnod; j++)
			if (!graph[p][pOld[j]]) {
				count++;
				// Save position of potential candidate
				pos = j;
			}
		// Test new minimum
		if (count < minnod) {
			fixp = p;
			minnod = count;
			if (i<ne)
				s = pos;
			else {
				s = i;
				// pre-increment
				nod = 1;
			}
		}
	}
	// If fixed point initially chosen from candidates then
	// number of diconnections will be preincreased by one
	// Backtrackcycle
	for (nod = minnod + nod; nod >= 1; nod--) {
		// Interchange
		p = pOld[s];
		pOld[s] = pOld[ne];
		sel = pOld[ne] = p;
		// Fill new set "not"
		newne = 0;
		for (i = 0; i < ne; i++)
			if (graph[sel][pOld[i]])
				newSet[newne++] = pOld[i];

		// Fill new set "cand"
		newce = newne;
		for (i = ne + 1; i<ce; i++)
			if (graph[sel][pOld[i]])
				newSet[newce++] = pOld[i];

		// Add to compsub
		compsub.add(sel);
		if (newce == 0) {
			/*****************************************
			* found a max clique: RECORD OR PRINT *
			*****************************************/
			if (compsub.size >= GlobalConstant::G_GOURP_QUERY_CLIQUE_MINI_SIZE) {
				addToCliques();
			}
			//compsub.print();
		}
		else if (newne < newce)
			bkv2(newSet, newne, newce);

		// Remove from compsub
		compsub.remove();

		// Add to "not"
		ne++;
		if (nod > 1)
			// Select a candidate disconnected to the fixed point
			for (s = ne; graph[fixp][pOld[s]]; s++)
				;
		// nothing but finding s

	} /* Backtrackcycle */
	free(newSet);
}


void FindAllMaximalCliques::addToCliques() {

	cliques->push_back(std::vector<int>());
	std::vector<int> & newClique = (*cliques)[cliques->size() - 1];

	for (int i = 0; i < compsub.size; i++) {

		bool isChildQuery = false;
		int thisCliqueNode = compsub.node[i];

		for (int j = 0; j < compsub.size; j++) {
			
			if (j == i) {
				continue;
			}

			int innerCliqueNode = compsub.node[j];
			/*
			 * Exclude the containment child queries. If q1  contains q2, then we need to remove q2 from this clique
			 */
			if ((*queryGraphVector)[thisCliqueNode].getNumberOfEdges() > (*queryGraphVector)[innerCliqueNode].getNumberOfEdges()) {
				if ((*patternContainmentMap)[innerCliqueNode].descendent.find(thisCliqueNode) != (*patternContainmentMap)[innerCliqueNode].descendent.end()) {
					isChildQuery = true;
					break;
				}
			}
		}
		if (!isChildQuery) {
			newClique.push_back(thisCliqueNode);
		}
	}
	/*
	 * If the clique size is less than the threshold, we need to remove it
	 */
	if (newClique.size() < GlobalConstant::G_GOURP_QUERY_CLIQUE_MINI_SIZE) {
		cliques->pop_back();
	}
}



FindAllMaximalCliques::nodeSet::nodeSet(void) {
	N = 0;
	size = 0;
}

FindAllMaximalCliques::nodeSet::~nodeSet(void) {
	if (N != 0)
		free(node);
}

void FindAllMaximalCliques::nodeSet::nodeInit(int NInput) {
	N = NInput;
	node = (int *)malloc(N*sizeof(int));
	size = 0;
}

void FindAllMaximalCliques::nodeSet::add(int nodeA) {
	node[size++] = nodeA;
}

void FindAllMaximalCliques::nodeSet::remove(void) {
	size--;
}

void FindAllMaximalCliques::nodeSet::print(void) {
	int i;
	printf(" the size of clique : %d - [", size);
	for (i = 0; i < size; i++) {
		if (i < size - 1)
			printf("%d,", node[i]);
		else printf("%d ]\n", node[i]);
	}
}


