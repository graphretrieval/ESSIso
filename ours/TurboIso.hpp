#include <map>
#include <vector>
#include <set>
#include <chrono>

#include "Graph.hpp"

class TurboIso {
private:
    std::chrono::steady_clock::time_point startTime;
    int nCall;
    bool needReturn;
    std::map<int, std::set<int> > candidates;
    std::map<int, int> mapping;
    std::map<int, int> reverseMapping;

private:
	class NECNode {
	public:
		int vertexId;
		std::vector<int> childList;
		int parent;
		int id;
		int label;
		~NECNode(){
			childList = std::vector<int>();
		}
	};
private:
	int startQueryVertex = -1;
	std::map<std::pair<int, int>, std::set<int>> CR;
	std::vector<NECNode> necTree;
	std::vector<int> queryMatchingSuquence; //vertexId, parentId;

public:
    std::vector<std::map<int,int>> allMappings;
    Graph dataGraph, queryGraph;
    int maxEmbedding;
    std::map<int,int> largestMapping;
    int maxRCall;
    float timeOut;

public:
	std::vector<std::map<int,int>> getAllSubGraphs(const Graph & pdataGraph, const Graph &pqueryGraph, int maxRCall_, int maxEmbedding_);

private:

	bool exploreCR(int u, int parentMappedNecTree, std::vector<int> & subRegionCandidates);
	int chooseStartVertex();
	bool degreeFilter(int u, int v);
	void rewriteToNecTree();
	void computeMatchingOrder();
	void devideNoTreeEdges(int u, int numberOfNoTreeEdges, float * queryVertexScore);
	void getOrderByBFSScore(int u, float * queryVertexScore, std::vector<int> & nextVertex);
	NECNode * nextQueryVertex();
	bool isJoinable(int u, int v);
	void subgraphSearch();
	void updateState(int u, int v);
    void restoreState(int u, int v);

};
