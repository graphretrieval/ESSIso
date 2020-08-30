#include "FindMaximalDisjointConnCliques.h"
#include "FindMaximumForestClique.h"

using namespace std;

void FindMaximalDisjointConnCliques::findMaximalDisjointConnCliques(bool ** graphMatrix, int graphSize, std::vector<std::vector<int>> & similiarQueryGroups)
{
	FindMaximumForestClique findMaximumForestClique(graphMatrix, graphSize, 0.025f);

	int forestSize;
	int * maxForest;
	findMaximumForestClique.mcqdyn(maxForest, forestSize);

	bool * flags = new bool[forestSize];
	for (int i = 0; i < forestSize; i++) {
		flags[i] = false;
	}

	for (int i = 0; i < forestSize; i++) {
		if (flags[i]) {
			continue;
		}

		similiarQueryGroups.push_back(vector<int>());
		vector<int> & clique = similiarQueryGroups[similiarQueryGroups.size() - 1];

		flags[i] = true;
		clique.push_back(maxForest[i]);
		for (int j = 0; j < forestSize; j++) {
			if (flags[j]) {
				continue;
			}
			for (vector<int>::iterator vertexIterator = clique.begin(); vertexIterator != clique.end(); vertexIterator++) {
				if (graphMatrix[*vertexIterator][maxForest[j]]) {
					flags[j] = true;
					clique.push_back(maxForest[j]);
					break;
				}
			}
		}
	}

}