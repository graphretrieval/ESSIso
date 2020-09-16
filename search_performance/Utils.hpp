#include <vector>
#include <map>
#include <set>

using namespace std;

namespace ProjectEmbedding {
    set<map<int,int>> crossProjectEmbedding(vector<map<int,int>> resultsGraph_A2B, map<int,int> mapNode_B2C);
    set<map<int,int>> projectEmbedding(set<map<int,int>> resultsGraph_A2B, set<int> nodes_A);
    float distance (vector<float> vec1, vector<float> vec2);
}
