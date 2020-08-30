#pragma once
#ifndef FIND_MAXIMUM_CONNECTED_CLIQUES_H
#define FIND_MAXIMUM_CONNECTED_CLIQUES_H

class FindMaximumConnectedClique {
public:
	/*
	* Based on the FindMaximumClique computed a maximum disconnected clique forest,
	* we split the forest into seperated cliques each of whom is connected
	* in the seperated cliques, return the clique with the largest number of vertices. 
	* @return int * & maximumConnectedClique, int & maximumConnectedCliqueSize
	*/
	static void findMaximumConnectedClique(bool ** graphMatrix, int graphSize, int * & maximumConnectedClique, int & maximumConnectedCliqueSize);
};

#endif