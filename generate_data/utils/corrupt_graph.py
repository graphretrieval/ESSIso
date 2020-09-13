import networkx as nx
import random 

def remove_node(subgraph,n_nodes=1):
    mapping = {node:node for i, node in enumerate(subgraph.nodes())}
    subG = nx.relabel_nodes(subgraph,mapping)
    nodes=[]
    prob=[]
    for node in subG.nodes():
        nodes.append(node)
        prob.append(subG.degree(node))
        
    removed_node = random.sample(nodes, min(n_nodes,len(nodes)))
    subG.remove_nodes_from(removed_node)

    return subG 

def remove_edge(subgraph,n_edges=1):
    mapping = {node:node for i, node in enumerate(subgraph.nodes())}
    subG = nx.relabel_nodes(subgraph,mapping)
    removed_edges= [edge for edge in subG.edges()]
    removed_edge = random.sample(removed_edges, min(n_edges, len(removed_edges)))
    subG.remove_edges_from(removed_edge)
    return subG 

def add_node(subgraph,n_edges=1):

    return subgraph 

def add_edge(subgraph,n_edges=1):
    mapping= {node:node for i, node in enumerate(subgraph.nodes())}
    subG = nx.relabel_nodes(subgraph,mapping)
    added_edges = []
    for i in subG.nodes():
        for j in subG.nodes():
            if i==j or (i,j) in subG.edges(): continue
            added_edges.append((i,j))
    added_edge = random.choice(added_edges)
    subG.add_edges_from([added_edge])
    return subG 