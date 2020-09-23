"""
    Prepare data to test with C++ code

"""

import torch
import random 
import numpy as np 
from tqdm import tqdm 
from scipy.spatial.distance import cdist,cosine
from scipy.optimize import linear_sum_assignment
import time 
from utils.corrupt_graph import remove_edge, remove_node, add_edge, add_node
from networkx.algorithms import isomorphism
from python_emb import * 
from sklearn.cluster import KMeans

def create_dimas_graph_format(graph, id_map, embedding, graph_index, multi2single_label, family_index=0, weighted_sum=False):
    inverted_id_map = {idx:node for node,idx in id_map.items()}
    lines = ['p edge {} {} {} {}\n'.format(len(graph.nodes), len(graph.edges), embedding.shape[-1], family_index)]
    if weighted_sum:
        weighted = np.array([ [ graph.degree(inverted_id_map[n]) ] for n in range(len(inverted_id_map))])
        embedding = (embedding*weighted).sum(0)/np.sum(weighted)
    else:
        embedding = embedding.mean(0)
    lines.append(' '.join(map(str, embedding)) + '\n')
    for n in range(len(inverted_id_map)):
        label = multi2single_label[tuple(graph.nodes[inverted_id_map[n]]['label'])]
        lines.append('n {} {}\n'.format(n, label))
    for u, v in graph.edges():
        lines.append('e {} {}\n'.format(id_map[u], id_map[v]))
    return lines

def create_dimas_graph_format_node_embedding(graph, id_map, embedding, graph_index, multi2single_label):
    inverted_id_map = {idx:node for node,idx in id_map.items()}
    lines = ['p edge {} {} {}\n'.format(len(graph.nodes), len(graph.edges), embedding.shape[-1])]
    lines.append(' '.join(map(str, embedding.mean(0))) + '\n')
    for n in range(len(inverted_id_map)):
        label = multi2single_label[tuple(graph.nodes[inverted_id_map[n]]['label'])]
        lines.append('n {} {}\n'.format(n, label))
        lines.append('{}\n'.format( ' '.join([str(entry) for entry in embedding[n].tolist()]) ) )
    for u, v in graph.edges():
        lines.append('e {} {}\n'.format(id_map[u], id_map[v]))
    return lines
    
def create_graph_format(graph, id_map, embedding, graph_index, multi2single_label):
    inverted_id_map = {idx:node for node,idx in id_map.items()}
    lines = ['t # {}\n'.format(graph_index)]
    for v in range(len(inverted_id_map)):
        # onehot_label = graph.nodes[inverted_id_map[v]]['label']
        # label = onehot_label.index(1)
        label = multi2single_label[tuple(graph.nodes[inverted_id_map[v]]['label'])]
        lines.append('v {} {}\n'.format(v, label))
        lines.append('{}\n'.format( ' '.join([str(entry) for entry in embedding[v].tolist()]) ) )
    for u, v in graph.edges():
        lines.append('e {} {} 0\n'.format(id_map[u], id_map[v]))
    return lines



args = parse_args()
print(args)

random.seed(args.seed)
np.random.seed(args.seed)
torch.manual_seed(args.seed)
torch.cuda.manual_seed(args.seed)

ori_emb_model, ori_graph_emb, ori_graph_data = get_model_and_data(args)

# data_lines = create_graph_format(ori_graph_data.G, ori_graph_data.id_map, ori_graph_emb, 0, ori_graph_data.multi2single_label)
# if not os.path.isdir(args.embedding_model):
#     os.makedirs(args.embedding_model)
# with open(os.path.join(args.embedding_model, 'd.graph'), 'w') as f:
#     for line in data_lines:
#         f.write(line)

query_machine = GraphQuery(ori_emb_model, 
        ori_graph_emb,
        ori_graph_data.G,
        ori_graph_data.id_map,
        ori_graph_data.feats,
        ori_graph_data.raw_feats,
        ori_graph_data.full_adj,
        ori_graph_data.deg)

def get_embedding_subgraph(subgraph):
    sub_id_map, sub_raw_feats, all_sub_adj, sub_degree = query_machine.create_subgraph_map(subgraph)
    embedding_subgraph = query_machine.embedding_subgraph(sub_raw_feats, all_sub_adj, sub_degree)
    embedding_subgraph = embedding_subgraph.detach().cpu().numpy()
    
    return embedding_subgraph, sub_id_map


def stat(node):
    level1 = []
    level2 = []
    for i in ori_graph_data.G.neighbors(node):
        level1.append(i)
    for i in level1:
        for j in ori_graph_data.G.neighbors(i):
            if j not in level1 and j!= node:
                level2.append(j)
#     print(len(level1), len(level2))
    allnodes = [node]+level1+level2
    return allnodes

def stat_(node, n_nodes):
    level1 = []
    level2 = []
    for i in ori_graph_data.G.neighbors(node):
        level1.append(i)
    level1 = random.sample(level1, min(len(level1), int(n_nodes/2)))
    for i in level1:
        for j in ori_graph_data.G.neighbors(i):
            if j not in level1 and j!= node:
                level2.append(j)
    level2 = random.sample(level2, min(len(level2), n_nodes-1-len(level1)))
#     print(len(level1), len(level2))
    return [node] + level1+ level2

import os
# filename = 'new_ori_graph_emb_{}.npy'.format(args.prefix.split('/')[-1])
# if not os.path.isfile(filename):
#     new_ori_graph_emb = np.zeros_like(ori_graph_emb)
#     for i in tqdm(ori_graph_data.G.nodes()):
#         subgraph = ori_graph_data.G.subgraph(stat(i))
#         embedding_subgraph, sub_id_map = get_embedding_subgraph(subgraph)
#         new_ori_graph_emb[i] = embedding_subgraph[sub_id_map[i]]
#     np.save(filename, new_ori_graph_emb)
# else:
#     new_ori_graph_emb = np.load(filename)
new_ori_graph_emb = ori_graph_emb
print(new_ori_graph_emb.shape)
N_CLUSTERS = 50
for max_subgraph_nodes in [10, 15, 20, 25, 30]:
    args.max_subgraph_nodes=max_subgraph_nodes
    savedir = os.path.join(args.embedding_model, '{}_nnode{}_k{}_coldstart'.format(args.prefix.split('/')[-1], args.max_subgraph_nodes, N_CLUSTERS))
    if not os.path.isdir(savedir):
        os.makedirs(savedir)

    with open(os.path.join(savedir, 'd.dimas'), 'w') as f:
        for line in create_dimas_graph_format(ori_graph_data.G, ori_graph_data.id_map, new_ori_graph_emb, 0, ori_graph_data.multi2single_label):
            f.write(line)

    with open(os.path.join(savedir, 'd.graph'), 'w') as f:
        for line in create_graph_format(ori_graph_data.G, ori_graph_data.id_map, new_ori_graph_emb, 0, ori_graph_data.multi2single_label):
            f.write(line)

    # most_common_label = Counter(np.squeeze(ori_graph_data.true_labels).tolist()).most_common(1)[0][0]
    # most_common_nodes = np.where(np.squeeze(ori_graph_data.true_labels) == most_common_label)[0]
    # core_nodes = random.sample(most_common_nodes.tolist(), args.num_subgraphs)

    subgraph_queries = []
    embedding_subgraphs = []
    mean_embedding_subgraphs = []
    for core_node in [node for node in query_machine.ori_graph]:
        sampled_node = stat_(core_node, args.max_subgraph_nodes)
        subgraph = ori_graph_data.G.subgraph(sampled_node)
        if len(subgraph) < args.max_subgraph_nodes:
            continue
        if nx.is_connected(subgraph):
            sub_id_map, sub_raw_feats, all_sub_adj, sub_degree = query_machine.create_subgraph_map(subgraph)

            embedding_subgraph = query_machine.embedding_subgraph(sub_raw_feats, all_sub_adj, sub_degree)
            embedding_subgraph = embedding_subgraph.detach().cpu().numpy()
            if not np.isnan(embedding_subgraph).any():
                subgraph_queries.append(subgraph)
                embedding_subgraphs.append(embedding_subgraph)
                mean_embedding_subgraphs.append(embedding_subgraph.mean(0))

    mean_embedding_subgraphs = np.array(mean_embedding_subgraphs)
    kmeans = KMeans(n_clusters=N_CLUSTERS).fit(mean_embedding_subgraphs)
    centroids = kmeans.cluster_centers_
    
    final_subgraph_queries = []
    final_embedding_subgraphs = []
    for i in range(N_CLUSTERS):
        best_id = np.argmin(((kmeans.cluster_centers_[i]-mean_embedding_subgraphs)**2).sum(1))
        final_subgraph_queries.append(subgraph_queries[i])
        final_embedding_subgraphs.append(embedding_subgraphs[i])

    query_lines = [] 
    dimas_query_lines = [] 
    dimas_query_lines_weighted = [] 
    for i, subgraph in tqdm(enumerate(final_subgraph_queries), desc="gen embedding subgraph"):
        sub_id_map, sub_raw_feats, all_sub_adj, sub_degree = query_machine.create_subgraph_map(subgraph)

        embedding_subgraph = query_machine.embedding_subgraph(sub_raw_feats, all_sub_adj, sub_degree)
        embedding_subgraph = embedding_subgraph.detach().cpu().numpy()
        if not np.isnan(embedding_subgraph).any():
            dimas_query_lines.append(create_dimas_graph_format(subgraph, sub_id_map, embedding_subgraph, i, ori_graph_data.multi2single_label, -1))
            dimas_query_lines_weighted.append(create_dimas_graph_format(subgraph, sub_id_map, embedding_subgraph, i, ori_graph_data.multi2single_label, -1, True))
            query_lines += create_graph_format(subgraph, sub_id_map, embedding_subgraph, i, ori_graph_data.multi2single_label)

    if not os.path.isdir(os.path.join(savedir, 'query')):
        os.mkdir(os.path.join(savedir, 'query'))
    if not os.path.isdir(os.path.join(savedir, 'query_weighted')):
        os.mkdir(os.path.join(savedir, 'query_weighted'))
    for i, query_line in enumerate(dimas_query_lines):
        with open(os.path.join(savedir,  'query' , 'q{:04d}.dimas'.format(i)), 'w') as f:
            for line in query_line:
                f.write(line)
    for i, query_line in enumerate(dimas_query_lines_weighted):
        with open(os.path.join(savedir,  'query_weighted' , 'q{:04d}.dimas'.format(i)), 'w') as f:
            for line in query_line:
                f.write(line)
    with open(os.path.join(savedir, 'q.graph'), 'w') as f:
        for line in query_lines:
            f.write(line)