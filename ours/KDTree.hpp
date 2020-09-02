#ifndef KDTREE_H

#define KDTREE_H
// A C++ program to demonstrate delete in K D tree 
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include<bits/stdc++.h> 
#include <limits>

const int k = 128; 

// A structure to represent node of kd tree 
struct Node 
{ 
	std::vector<float> point;
    int index;
	Node *left, *right; 
}; 

// A method to create a node of K D tree 
struct Node* newNode(int index, std::vector<float> vector) 
{ 
	struct Node* temp = new Node; 

    temp->index = index;
    temp->point = vector;

	temp->left = temp->right = NULL; 
	return temp; 
} 

// Inserts a new node and returns root of modified tree 
// The parameter depth is used to decide axis of comparison 
Node *insertRec(Node *root, int index, std::vector<float> vector, unsigned depth) 
{ 
	// Tree is empty? 
	if (root == NULL) 
		return newNode(index, vector); 

	// Calculate current dimension (cd) of comparison 
	unsigned cd = depth % vector.size(); 

	// Compare the new point with root on current dimension 'cd' 
	// and decide the left or right subtree 
	if (vector[cd] < (root->point[cd])) 
		root->left = insertRec(root->left, index, vector, depth + 1); 
	else
		root->right = insertRec(root->right, index, vector, depth + 1); 

	return root; 
} 

// Function to insert a new point with given point in 
// KD Tree and return new root. It mainly uses above recursive 
// function "insertRec()" 
Node* insertKDTree(Node *root, int index, std::vector<float> vector) 
{ 
	return insertRec(root, index, vector, 0); 
} 

// A utility function to find minimum of three integers 
Node *minNode(Node *x, Node *y, Node *z, int d) 
{ 
	Node *res = x; 
	if (y != NULL && y->point[d] < res->point[d]) 
	res = y; 
	if (z != NULL && z->point[d] < res->point[d]) 
	res = z; 
	return res; 
} 

// Recursively finds minimum of d'th dimension in KD tree 
// The parameter depth is used to determine current axis. 
Node *findMinRec(Node* root, int d, unsigned depth) 
{ 
	// Base cases 
	if (root == NULL) 
		return NULL; 

	// Current dimension is computed using current depth and total 
	// dimensions (k) 
	unsigned cd = depth % root->point.size(); 

	// Compare point with root with respect to cd (Current dimension) 
	if (cd == d) 
	{ 
		if (root->left == NULL) 
			return root; 
		return findMinRec(root->left, d, depth+1); 
	} 

	// If current dimension is different then minimum can be anywhere 
	// in this subtree 
	return minNode(root, 
			findMinRec(root->left, d, depth+1), 
			findMinRec(root->right, d, depth+1), d); 
} 

// A wrapper over findMinRec(). Returns minimum of d'th dimension 
Node *findMin(Node* root, int d) 
{ 
	// Pass current level or depth as 0 
	return findMinRec(root, d, 0); 
} 

// A utility method to determine if two Points are same 
// in K Dimensional space 
bool arePointsSame(std::vector<float> point1, std::vector<float> point2) 
{ 
	// Compare individual pointinate values 
	for (int i = 0; i < point1.size(); ++i) 
		if (point1[i] != point2[i]) 
			return false; 

	return true; 
} 

float distance(std::vector<float> a, std::vector<float> b) {
	float dist = 0;
	for (int i = 0; i< a.size(); i++) {
		dist += (a[i] - b[i])*(a[i] - b[i]);
	}
	return dist;
}
// Searches a Point represented by "point[]" in the K D tree. 
// The parameter depth is used to determine current axis. 
void nearestSearchRec(Node* root, std::vector<float> point, unsigned depth,  int* nearestIndex, float* nearestDistance, std::unordered_map<int, float>* distanceMap) 
{ 
    // Base cases 
    if (root == NULL) 
		return;  
    float dist = distance(root->point, point);
    distanceMap->insert({root->index, dist});
    // Current dimension is computed using current depth and total 
    // dimensions (k) 
    unsigned cd = depth % root->point.size(); 
	float dx = root->point[cd] - point[cd];
	float dx2 = dx*dx;
	if (*nearestIndex == -1 || dist < *nearestDistance) {
		*nearestDistance = dist;
		*nearestIndex = root->index;
	}

	if (*nearestDistance == 0) return;
  
    // Compare point with root with respect to cd (Current dimension) 
    if (dx>0) {
		nearestSearchRec(root->left, point, depth + 1, nearestIndex, nearestDistance, distanceMap); 
	}
	else {
		nearestSearchRec(root->right, point, depth + 1, nearestIndex, nearestDistance, distanceMap); 
	}
		
	if (dx2 >= *nearestDistance) return;

    if (dx>0) {
		nearestSearchRec(root->right, point, depth + 1, nearestIndex, nearestDistance, distanceMap); 
	}
        
	else {
		nearestSearchRec(root->left, point, depth + 1, nearestIndex, nearestDistance, distanceMap); 
	}		
} 

// Searches a Point in the K D tree. It mainly uses 
// searchRec() 
void nearestSearch(Node* root, std::vector<float> point, int* nearestIndex, float* nearestDistance, std::unordered_map<int, float>* distanceMap) 
{ 
    // Pass current depth as 0 
    nearestSearchRec(root, point, 0, nearestIndex, nearestDistance, distanceMap); 
} 

void kNearestSearchRec(Node* root, std::vector<float> point, unsigned depth, std::priority_queue<std::pair<float, int>>* distanceQueue, std::unordered_map<int, float>* distanceMap, int k) 
{ 
    // Base cases 
    if (root == NULL) 
		return;  
    float dist = distance(root->point, point);
    distanceMap->insert({root->index, dist});
    // Current dimension is computed using current depth and total 
    // dimensions (k) 
    unsigned cd = depth % root->point.size(); 
	float dx = root->point[cd] - point[cd];
	float dx2 = dx*dx;
	// std::cout << "Consider " << root->index <<":";
	if (distanceQueue->size() < k) {
		distanceQueue->push({dist, root->index});
		// std::cout << "Push " << dist << "," << root->index <<std::endl;
	} else if (dist < distanceQueue->top().first) {
		distanceQueue->pop();
		distanceQueue->push({dist, root->index});
		// std::cout << "Push " << dist << "( <" << distanceQueue->top().first << ")" << "," << root->index <<std::endl;
	}
	
    // Compare point with root with respect to cd (Current dimension) 
    if (dx>0) {
		kNearestSearchRec(root->left, point, depth + 1, distanceQueue, distanceMap,k); 
	}
	else {
		kNearestSearchRec(root->right, point, depth + 1, distanceQueue, distanceMap, k); 
	}
		
	if (dx2 >= distanceQueue->top().first && distanceQueue->size() >= k) return;
    if (dx>0) {
		kNearestSearchRec(root->right, point, depth + 1, distanceQueue, distanceMap, k);
	}
        
	else {
		kNearestSearchRec(root->left, point, depth + 1, distanceQueue, distanceMap,k); 
	}		
}   

// Searches a Point in the K D tree. It mainly uses 
// searchRec() 
void kNearestSearch(Node* root, std::vector<float> point, std::priority_queue<std::pair<float, int>>* distanceQueue, std::unordered_map<int, float>* distanceMap, int k) 
{ 
    // Pass current depth as 0 
    kNearestSearchRec(root, point, 0, distanceQueue, distanceMap, k); 
} 


bool searchRec(Node* root, std::vector<float> point, unsigned depth) 
{ 
    // Base cases 
    if (root == NULL) 
        return false; 
    if (arePointsSame(root->point, point)) 
        return true; 
  
    // Current dimension is computed using current depth and total 
    // dimensions (k) 
    unsigned cd = depth % root->point.size(); 
  
    // Compare point with root with respect to cd (Current dimension) 
    if (point[cd] < root->point[cd]) 
        return searchRec(root->left, point, depth + 1); 
  
    return searchRec(root->right, point, depth + 1); 
} 
  
// Searches a Point in the K D tree. It mainly uses 
// searchRec() 
bool search(Node* root, std::vector<float> point) 
{ 
    // Pass current depth as 0 
    return searchRec(root, point, 0); 
}

// // Copies point p2 to p1 
void copyPoint(int & index1, std::vector<float> & point1, int index2, std::vector<float> point2) 
{ 
	index1 = index2;
    for (int i=0; i<point2.size(); i++) 
        point1[i] = point2[i]; 
} 

// Function to delete a given point 'point[]' from tree with root 
// as 'root'. depth is current depth and passed as 0 initially. 
// Returns root of the modified tree. 
Node *deleteNodeRec(Node *root, int index, std::vector<float> point, int depth) 
{ 
	// Given point is not present 
	if (root == NULL) 
		return NULL; 

	// Find dimension of current node 
	int cd = depth % root->point.size(); 

	// If the point to be deleted is present at root 
	if (root->index == index) 
	{ 
		// 2.b) If right child is not NULL 
		if (root->right != NULL) 
		{ 
			// Find minimum of root's dimension in right subtree 
			// Node *min = findMin(root->right, cd); 
			Node *min = findMinRec(root->right, cd, depth+1); 
			// Copy the minimum to root 
			copyPoint(root->index, root->point, min->index, min->point); 

			// Recursively delete the minimum 
			root->right = deleteNodeRec(root->right, min->index, min->point, depth+1); 
		} 
		else if (root->left != NULL) // same as above 
		{ 
			// Node *min = findMin(root->left, cd); 
			Node *min = findMinRec(root->left, cd, depth+1); 
			copyPoint(root->index, root->point, min->index, min->point); 
			root->right = deleteNodeRec(root->left, min->index, min->point, depth+1); 
			root->left = NULL;
		} 
		else // If node to be deleted is leaf node 
		{ 
			delete root; 
			return NULL; 
		} 
		return root; 
	} 

	// 2) If current node doesn't contain point, search downward 
	if (point[cd] < root->point[cd]) 
		root->left = deleteNodeRec(root->left, index, point, depth+1); 
	else
		root->right = deleteNodeRec(root->right, index, point, depth+1); 
	return root; 
} 

// Function to delete a given point from K D Tree with 'root' 
Node* deleteKDTree(Node *root, int index, std::vector<float> point) 
{ 
// Pass depth as 0 
return deleteNodeRec(root, index, point, 0); 
} 

#endif