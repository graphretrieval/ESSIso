#include "FindMaximumConnectedClique.h"
#include "FindMaximumForestClique.h"
#include <vector>
#include <chrono>

using namespace std;

void FindMaximumConnectedClique::findMaximumConnectedClique(bool ** graphMatrix, int graphSize, int * & maximumConnectedClique, int & maximumConnectedCliqueSize)
{
	cout << "findMaximumForestClique"<< endl;
	std::chrono::steady_clock::time_point start;
	start = std::chrono::steady_clock::now();
	FindMaximumForestClique findMaximumForestClique(graphMatrix, graphSize, 0.025f);
	cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << endl;

	int forestSize;
	int * maxForest;
	cout << "mcqdyn" << endl;
	start = std::chrono::steady_clock::now();
	findMaximumForestClique.mcqdyn(maxForest, forestSize);
	cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << endl;

	cout << forestSize << endl;
	bool * flags = new bool[forestSize];
	for (int i = 0; i < forestSize; i++) {
		flags[i] = false;
	}

	vector<vector<int>> cliques;

	start = std::chrono::steady_clock::now();
	for (int i = 0; i < forestSize; i++) {
		if (flags[i]) {
			continue;
		}

		cliques.push_back(vector<int>());
		vector<int> & clique = cliques[cliques.size() - 1];

		flags[i] = true;
		clique.push_back(maxForest[i]);
		for (int j = 0; j < forestSize; j++) {
			if (flags[j]) {
				continue;
			}
			cout << clique.size() << endl;
			for (vector<int>::iterator vertexIterator = clique.begin(); vertexIterator != clique.end(); vertexIterator++) {
				if (graphMatrix[*vertexIterator][maxForest[j]]) {
					flags[j] = true;
					clique.push_back(maxForest[j]);
					break;
				}
			}
		}
	}
	cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << endl;

	int maximumSize = 0;
	int maximumIndex = 0;

	for (size_t cliqueIndex = 0; cliqueIndex < cliques.size(); cliqueIndex++) {
		if (cliques[cliqueIndex].size() > maximumSize) {
			maximumSize = cliques[cliqueIndex].size();
			maximumIndex = cliqueIndex;
		}
	}

	maximumConnectedCliqueSize = maximumSize;
	maximumConnectedClique = new int[maximumConnectedCliqueSize];
	int vertexIndex = 0;
	for(vector<int>::iterator vertexIterator = cliques[maximumIndex].begin(); vertexIterator != cliques[maximumIndex].end(); vertexIterator++) {
		maximumConnectedClique[vertexIndex] = *vertexIterator;
		vertexIndex++;
	}
}
