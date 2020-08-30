#include"RdfMQO.h"
#include<cmath>
#include<set>

using namespace std;


RdfMQO::RdfMQO(std::vector<AdjacenceListsGRAPH> & pQueryGraphVector, AdjacenceListsGRAPH & pDataGraph, vector<std::vector<int>> & pQueryGroups) {
	queryGraphVector = pQueryGraphVector;
	dataGraph = pDataGraph;
	queryGroups = pQueryGroups;
}


void RdfMQO::kMeansGroupQueries()
{

	int kGroups = queryGraphVector.size() / 40; //according to their paper
	for (int i = 0; i < kGroups; i++) {
		queryGroups.push_back(vector<int>());
	}
	double ** simi = new double*[queryGraphVector.size()];
	int * assignedGroup = new int[queryGraphVector.size()];
	int kMeansIteration = 10; // for efficiency consideration, we iterate 4 times for the kmeans instead of wait it to converge


	for (size_t i = 0; i < queryGraphVector.size(); i++) {
		simi[i] = new double[queryGraphVector.size()];

		for (size_t j = i + 1; j < queryGraphVector.size(); j++) {
			AdjacenceListsGRAPH * graph1 = &queryGraphVector[i];
			AdjacenceListsGRAPH * graph2 = &queryGraphVector[j];

			double intersection = 0;
			set<int> * graph1LabelSet = graph1->getLabelSet();
			set<int> * graph2LabelSet = graph2->getLabelSet();
			for (set<int>::iterator labelIterator = graph1LabelSet->begin(); labelIterator != graph1LabelSet->end(); labelIterator++) {
				if (graph2LabelSet->find(*labelIterator) == graph2LabelSet->end()) {
					intersection++;
				}
			}
			simi[i][j] = intersection / (graph1LabelSet->size() + graph2LabelSet->size() - intersection);
			simi[j][i] = simi[i][j];
		}
	}

	vector<int> centroids;
	// randomly use the first k elements as centroids
	for (int i = 0; i < kGroups; i++) {
		centroids.push_back(i);
	}

	for (int kMeanIter = 0; kMeanIter < kMeansIteration; kMeanIter++) {
		// mark the centroid assignedGroup as -1 and others as -2
		for (vector<int>::iterator centroidIter = centroids.begin(); centroidIter != centroids.end(); centroidIter++) {
			assignedGroup[*centroidIter] = -1;
		}
		for (size_t i = 0; i < queryGraphVector.size(); i++) {
			if (assignedGroup[i] != -1) {
				assignedGroup[i] = -2;
			}
		}

		// assign each query to a centroid
		for (size_t i = 0; i < queryGraphVector.size(); i++) {
			if (assignedGroup[i] != -1) {
				double maxSimi = -1;
				int assignedCentroid = -1;
				for (vector<int>::iterator centroidIter = centroids.begin(); centroidIter != centroids.end(); centroidIter++) {
					if (maxSimi < simi[*centroidIter][i]) {
						maxSimi = simi[*centroidIter][i];
						assignedCentroid = *centroidIter;
					}
				}
				assignedGroup[i] = assignedCentroid;
			}
		}

		/*
		* for each group, recompute the centroid
		* there is no information provided in their paper in terms of how to choose the centroid.
		* we choose the centroid as the query whose distance with the old centroid closest to the mean
		*/
		for (vector<int>::iterator centroidIter = centroids.begin(); centroidIter != centroids.end(); centroidIter++) {
			double simiSum = 0;
			for (size_t i = 0; i < queryGraphVector.size(); i++) {
				if (assignedGroup[i] == *centroidIter) {
					simiSum += simi[*centroidIter][i];
				}
			}
			double simiMean = simiSum / (kGroups - 1);
			double miniDif = 0.1; //DBL_MAX;
			int newCentroid = -1;
			for (size_t i = 0; i < queryGraphVector.size(); i++) {
				if (assignedGroup[i] == *centroidIter) {
					double dif = abs(simi[*centroidIter][i] - simiMean);
					if (dif < miniDif) {
						newCentroid = i;
						miniDif = dif;
					}
				}
			}
			// reset the centroids
			(*centroidIter) = newCentroid;
		}
	}

	/*
	* retrieve the groups from kMeans
	*/
	for (size_t i = 0; i < queryGraphVector.size(); i++) {
		if (assignedGroup[i] == -1) {
			queryGroups[i].push_back(i);
		}
		else {
			queryGroups[assignedGroup[i]].push_back(i);
		}
	}

	/*
	* release memory
	*/
	for (size_t i = 0; i < queryGraphVector.size(); i++) {
		delete[] simi[i];
	}
	delete[] simi;

	delete[] assignedGroup;
}

void RdfMQO::refineQueryGroups(){

}
