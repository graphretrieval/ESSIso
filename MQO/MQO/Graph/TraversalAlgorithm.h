#ifndef TRAVERSAL_ALGORITHM
#define TRAVERSAL_ALGORITHM

#include<vector>
#include<queue>
#include<stack>
#include"AdjacenceListsGraph.h"

using std::vector;
using std::queue;
using std::stack;

struct TreeNode{
public:
	int id;
	int parentId;

	vector<int> previousMatchedAdj; // used by QuickSI QIsequence

	TreeNode(int vertexId, int vertexParentId):id (vertexId),parentId (vertexParentId){}

	TreeNode(int vertexId):id (vertexId){}
	
	TreeNode(){}

};

class TraversalAlgorithm{
public:
	static vector<TreeNode> BFS_VertexSequence(const AdjacenceListsGRAPH*, int startVertexId); // vertexId,vertexParentId;
	static vector<TreeNode> DFS_VertexSequence(const AdjacenceListsGRAPH*, int startVertexId); // vertexId,vertexParentId;
};

#endif