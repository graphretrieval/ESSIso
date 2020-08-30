"""
    Set of functions used to search a subgraph
"""

import networkx as nx
from networkx.generators.random_graphs import fast_gnp_random_graph
from collections import defaultdict
import random

def NextQueryVertex(M, subgraph):
    nodes = [node for node in subgraph.nodes()]
    for node_idx in range(len(nodes)):
        if subgraph.graph['bfs_order'][node_idx] not in M:
            return subgraph.graph['bfs_order'][node_idx]
            
def RefineCandidates(u, M, candidates, subG, G):
    candidate = candidates[u]
    return [v for v in candidate if subG.degree(u) <= G.degree(v) and v not in M.values()]

def IsJoinable(u,v,M,subG,G):
    for matched_u in M:
        if matched_u not in subG.neighbors(u):
            continue
        else:
            # has_connection = True
            matched_v = M[matched_u]
            if (v, matched_v) not in G.edges():
                return False
    return True

def SubgraphSearch(M,G,subG, candidates, final_results, k=10):

    if len(M) == len(subG.nodes()):
        final_results.append(M.copy())
        if len(final_results) >= k: return final_results
    else:
        u = NextQueryVertex(M, subG)
        C_R = RefineCandidates(u, M, candidates, subG, G)
        for v in C_R:
            if v in M.values():
                continue
            is_joinable = IsJoinable(u,v,M,subG,G)
            if is_joinable:
                # UpdateState
                M[u]=v
                final_results = SubgraphSearch(M,G,subG,candidates, final_results, k)
                if len(final_results) >= k: return final_results
                # RestoreState
                del M[u]
    return final_results
