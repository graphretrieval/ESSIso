import networkx as nx
import random 
from scipy.spatial.distance import cdist
from collections import defaultdict
import numpy as np
from utils.search_algo import SubgraphSearch
import time
import torch.nn.functional as F

def get_candidates(cached_G, query_G, cached_embeddings, query_embeddings, cached_node2id, node2id, threshold=0.1):
    cached_id2node = {v:k for k,v in cached_node2id.items()}
    id2node = {v:k for k,v in node2id.items()}

    pair_distances = cdist(query_embeddings, cached_embeddings, metric='cosine')

    candidates = defaultdict(dict)
    for i in range(query_embeddings.shape[0]):
        query_node = id2node[i]
        candidate_ids = np.argwhere(pair_distances[i]<=threshold).reshape((-1,))
        for candidate_id in candidate_ids:
            cached_node = cached_id2node[candidate_id]
            if query_G.nodes[query_node]['label'] == cached_G.nodes[cached_node]['label']:
                candidates[query_node][cached_node] = pair_distances[i][candidate_id]
    
    return candidates

class GraphQuery():
    def __init__(self, model, embeddings, G, id_map, feats, raw_feats, full_adj, deg):

        self.ori_emb_model = model

        # origraph
        self.ori_graph = G
        self.embeddings = embeddings
        self.id_map = id_map
        self.id_map_inverse = {v:k for k,v in id_map.items()}
        self.feats = feats
        self.raw_feats = raw_feats
        self.adj = full_adj 
        self.degree = deg

        # subgraph
        self.sub_graph = None
        self.sub_id_map = None
        self.sub_feats = None
        self.sub_adj = None
        self.sub_degree = None
    
    def add_new_nodes(self, n_nodes=50):
        n_origin_nodes = len(self.ori_graph.nodes)
        average_degree = int(np.array([self.ori_graph.degree(node) for node in self.ori_graph.nodes]).mean())

        adj_lists = self.adj
        degrees = self.degree
        degrees = np.append(degrees, np.zeros((n_nodes, )))

        new_nodes = [n_origin_nodes+i for i in range((n_nodes))]
        self.ori_graph.add_nodes_from(new_nodes)

        for new_node in new_nodes:
            adj_lists[new_node] = set()
        for new_node in new_nodes:
            random_nodes = np.random.choice(n_origin_nodes+n_nodes, average_degree)
            self.ori_graph.add_edges_from([(new_node, node) for node in random_nodes])
            adj_lists[new_node] = adj_lists[new_node].union(random_nodes)
            degrees[new_node] = len(random_nodes)

            for node in random_nodes:
                adj_lists[node].add(new_node)
                degrees[node] += 1

        for node in new_nodes:
            self.id_map[node] = node
            self.id_map_inverse[node] = node 

        idx = np.random.randint(n_origin_nodes, size=n_nodes)
        new_feats = self.feats[idx,:]
        new_raw_feats = new_feats[:,:]
        self.feats = np.vstack([self.feats, new_feats])
        self.raw_feats = np.vstack([self.raw_feats, new_raw_feats])
        
        for i in range(n_nodes):
            self.ori_graph.nodes[new_nodes[i]]['label'] = new_feats[i].tolist()

        self.ori_emb_model.set_params(adj_lists, degrees, self.feats)
        self.embeddings = F.normalize(self.ori_emb_model.aggregator(list(range(len(self.ori_graph.nodes))))).detach().cpu().numpy()

        return self.embeddings

    def create_subgraph_from_core(self, core_node, num_nodes):
        #print("Generating Subgraph...")
        nodes = []
        self.source_node = core_node
        nodes.append(core_node)
        n_tries = 0
        while num_nodes > len(nodes) and n_tries<50:
            added_node = random.choice(list(nodes))
            candidate_nodes = [node for node in self.ori_graph.neighbors(added_node) if node not in nodes]
            if len(candidate_nodes) == 0:
                n_tries+=1
                continue
            candidate_node = random.choice(candidate_nodes)
            nodes.append(candidate_node)
            
        self.sub_graph = self.ori_graph.subgraph(nodes)

        bfs_order = []
        for edge in list(nx.bfs_edges(self.sub_graph, self.get_centroid(self.sub_graph))):
            for n in edge:
                if n not in bfs_order:
                    bfs_order.append(n)
        self.sub_graph.graph['bfs_order'] = bfs_order
        self.sub_graph.graph['dfs_order'] = list(nx.dfs_preorder_nodes(self.sub_graph, source=self.get_centroid(self.sub_graph)))
        
        return self.sub_graph

    def create_subgraph_map(self, sub_graph=None):
        if sub_graph is None:
            sub_graph = self.sub_graph

        # create sub_graph_map:
        self.sub_id_map = {id:i for i, id in enumerate(sub_graph.nodes())}
        self.sub_id_map_inverse = {v:k for k, v in self.sub_id_map.items()}
        self.sub_feats = np.zeros((len(self.sub_id_map), self.feats.shape[1]))
        self.sub_raw_feats = np.zeros((len(self.sub_id_map), self.raw_feats.shape[1]))

        for i in range(len(self.sub_id_map)):
            id = self.sub_id_map_inverse[i]
            old_index = self.id_map[id]
            self.sub_feats[i] = self.feats[old_index]
            self.sub_raw_feats[i] = self.raw_feats[old_index]

        self.all_sub_adj = {}
        for nodeid in sub_graph.nodes():
            neighbors = np.array([self.sub_id_map[neighbor] for neighbor in sub_graph.neighbors(nodeid)])
            self.all_sub_adj[self.sub_id_map[nodeid]] = set(neighbors)
        self.sub_degree = np.array([sub_graph.degree(node) for node in sub_graph.nodes()])
         
        return self.sub_id_map, self.sub_raw_feats, self.all_sub_adj, self.sub_degree

    def query_exact_graph(self, sub_graph=None):
        if sub_graph is None:
            sub_graph = self.sub_graph

        candidates = defaultdict(list)
        # centroid_node = self.get_centroid(sub_graph)
        for node in sub_graph.nodes():
            for candidate_node in self.ori_graph.nodes():
                if sub_graph.nodes[node]['label'] == self.ori_graph.nodes[candidate_node]['label']:
                    candidates[node].append(candidate_node)

        M=dict()
        print({k:len(v) for k,v in candidates.items()})
        final_results = SubgraphSearch(M,self.ori_graph, sub_graph, candidates,[], 10)
        
        return final_results

    def query_graph(self, embeddings=None, embedding_subgraph=None, sub_graph=None, sub_id_map=None, n_candidates=10):
        if embeddings is None:
            embeddings = self.embeddings
        if sub_graph is None:
            sub_graph = self.sub_graph
        if sub_id_map is None:
            sub_id_map = self.sub_id_map

        candidates = defaultdict(list)
        centroid_node = self.get_centroid(sub_graph)
        a = 0
        for node in sub_graph.nodes():
            if node == centroid_node:
                # num_candidates = 5
                num_candidates = n_candidates
            else:
                num_candidates = n_candidates

            new_index = sub_id_map[node]
            #distance_matrix = ((embeddings - embedding_subgraph[new_index])**2).sum(axis = 1)
            distance_matrix = cdist(embedding_subgraph[new_index:new_index+1],embeddings,'cosine')[0]
            arg_sort_distance = np.argsort(distance_matrix)
            # arg_sort_distance = np.argwhere(distance_matrix<=0.1).reshape((-1,))
            #distance_matrix2 = cdist(embedding_subgraph[new_index:new_index+1],embeddings,'cosine')[0]
            for i in range(len(arg_sort_distance[:num_candidates])):
                candidate_node = self.id_map_inverse[arg_sort_distance[i]]
                if sub_graph.nodes[node]['label'] == self.ori_graph.nodes[candidate_node]['label'] and sub_graph.degree(node) <= self.ori_graph.degree(candidate_node):
                    candidates[node].append(self.id_map_inverse[arg_sort_distance[i]])
            if len(candidates[node]) == 0:
                a+=1
                for candidate_node in self.ori_graph.nodes():
                    if sub_graph.nodes[node]['label'] == self.ori_graph.nodes[candidate_node]['label'] and sub_graph.degree(node) <= self.ori_graph.degree(candidate_node):
                        candidates[node].append(candidate_node)
        print(a,{k:len(v) for k,v in candidates.items()})

        M=dict()
        stime = time.time()
        final_results = SubgraphSearch(M,self.ori_graph, sub_graph, candidates,[], 1)
        running_time = time.time() - stime
        # ground_truth = {node:node for node in sub_graph.nodes()}
        # acc = 0
        # for i in [dict(s) for s in set(frozenset(d.items()) for d in final_results)]:
        #     if i==ground_truth: 
        #         acc = 1
        if len(final_results) > 0: acc = 1
        else: acc = 0
        return acc, running_time

    def exact_query_graph(self, sub_graph=None, sub_id_map=None):
        candidates = defaultdict(list)

        for node in sub_graph.nodes():
            for candidate_node in self.ori_graph.nodes():
                if sub_graph.nodes[node]['label'] == self.ori_graph.nodes[candidate_node]['label']:
                    candidates[node].append(candidate_node)
        print({k:len(v) for k,v in candidates.items()})

        M=dict()
        stime = time.time()
        final_results = SubgraphSearch(M,self.ori_graph, sub_graph, candidates,[],1)
        running_time = time.time() - stime
        # ground_truth = {node:node for node in sub_graph.nodes()}
        # acc = 0
        # for i in [dict(s) for s in set(frozenset(d.items()) for d in final_results)]:
        #     if i==ground_truth: 
        #         acc = 1
        if len(final_results) > 0: acc=1
        else: acc=0
     
        return acc, running_time

    def query_nodes(self, embeddings=None, embedding_subgraph=None, sub_graph=None, sub_id_map=None):
        if embeddings is None:
            embeddings = self.embeddings
        if sub_graph is None:
            sub_graph = self.sub_graph
        if sub_id_map is None:
            sub_id_map = self.sub_id_map

        centroid_node = self.get_centroid(sub_graph)
        old_index = self.id_map[centroid_node]
        new_index = sub_id_map[centroid_node]

        distance_matrix = ((embeddings - embedding_subgraph[new_index])**2).sum(axis = 1)
        arg_sort_distance = np.argsort(distance_matrix)
    
        #print('num nodes source_graph, sub_graph: {}  {}'.format(len(embedding_subgraph),len(embeddings)))
        #print('mean distance: {:.4f}'.format(np.mean(distance_matrix)))
        #print('distance anchor: {:.4f}'.format(distance_matrix[old_index]))
        #print('min, max distance: {:.4f} {:.4f}'.format(distance_matrix[arg_sort_distance[0]], distance_matrix[arg_sort_distance[-1]]))

        centroid_rank = 0
        for i in range(len(arg_sort_distance)):
            if arg_sort_distance[i] == old_index:
                #print('rank anchor: {}'.format(i+1))
                #print('accuracy: {:.4f}'.format(1/(i+1)))
                centroid_rank = i+1
                print(distance_matrix[i])
                return centroid_rank 
       
    def get_centroid(self, sub_graph):
        """
        return node with max degree of subgraph
        """
        centroid_node = 0
        max_degree = 0
        for node in sub_graph.nodes():
            if sub_graph.degree(node) > max_degree:
                max_degree = sub_graph.degree(node)
                centroid_node = node
        return centroid_node
    
    def embedding_subgraph(self, sub_raw_feats=None, all_sub_adj=None, sub_degree=None):
        if sub_raw_feats is None:
            sub_raw_feats = self.sub_raw_feats
        if all_sub_adj is None:
            all_sub_adj = self.all_sub_adj
        if sub_degree is None:
            sub_degree = self.sub_degree

        self.ori_emb_model.set_params(all_sub_adj, sub_degree, sub_raw_feats)

        return F.normalize(self.ori_emb_model.aggregator(list(range(sub_raw_feats.shape[0]))),dim=1)

def normalize(embedding):
    return embedding / (np.sqrt((embedding**2).sum(axis = 1)).reshape(len(embedding),1))
