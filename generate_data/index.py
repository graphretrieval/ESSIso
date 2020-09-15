#!/usr/bin/env python
# coding: utf-8

# In[1]:


import torch
import random 
import numpy as np 
from tqdm import tqdm 
from scipy.spatial.distance import cdist, cosine
from scipy.optimize import linear_sum_assignment
from utils.corrupt_graph import remove_edge, remove_node, add_edge, add_node
from utils.query_machine import get_candidates
from python_emb import *
import torch.nn.functional as F
from scipy.spatial.distance import cosine
from scipy.stats import pearsonr, spearmanr
from collections import defaultdict
import time
from subprocess import Popen, PIPE
import pickle
import sys
import signal

# # GGSX

# In[2]:
def ggsx_feat(subgraph,k=6):
    counter_path_label = {}
    def pathSearch(graph: nx.Graph , node, n, path):
        ori_path = path[:]
        path.append(node)
        path_label = tuple(graph.nodes[i]['label'] for i in path)
        counter_path_label[path_label] = counter_path_label.get(path_label,0) +1
        if len(path) < n:
            for neigh in graph.neighbors(node):
                if neigh in path:
                    continue
                pathSearch(graph, neigh, n, path)
        del path[-1]
        assert path == ori_path

    path = []
    for node in subgraph.nodes():
        pathSearch(subgraph,node, k,path)
    return counter_path_label


# # MCS

# In[3]:


def select_label_class(future):
    labels = list(future.keys())
    values = [max(len(future[i][0]), len(future[i][1])) for i in labels]
    selected_label = labels[np.argmin(np.array(values))]
    return selected_label, future[selected_label]

def select_vertex(setG, g):
    listG = list(setG)
    values = [g.degree(i) for i in setG]
    return listG[np.argmax(np.array(values))]

def mcs(g,h):
    labelset_g = defaultdict(set)
    labelset_h = defaultdict(set)
    for node in g.nodes:
        labelset_g[g.nodes[node]['label']].add(node)
    for node in h.nodes:
        labelset_h[h.nodes[node]['label']].add(node)

    future = {}
    for label in set(labelset_g.keys()).intersection(labelset_h.keys()):
        future[str(label)+'-'] = [labelset_g[label], labelset_h[label]]
    M, incumbent = {}, {}

    def search(future):
        nonlocal incumbent, M, g,h
        if len(M) > len(incumbent):
            incumbent = M.copy()
        bound = len(M) + sum([ min(len(value[0]), len(value[1])) for value in future.values()])
        if bound <= len(incumbent):
            return 
        if len(future)==0:
            return
        selected_label, (setG, setH) = select_label_class(future)
        v = select_vertex(setG, g)
        for w in setH:
            future_ = {}
            for cur_label, (setG_, setH_) in future.items():
                setG__ = (setG_.intersection([node for node in g.neighbors(v)])).difference([v])
                setH__ = (setH_.intersection([node for node in h.neighbors(w)])).difference([w])
                if len(setG__) != 0 and len(setH__) != 0:
                    future_[cur_label+'1'] = [setG__, setH__]

                setG__ = (setG_.intersection([node for node in g.nodes if node not in g.neighbors(v)] )).difference([v])
                setH__ = (setH_.intersection([node for node in h.nodes if node not in h.neighbors(w)] )).difference([w])
                if len(setG__) != 0 and len(setH__) != 0:
                    future_[cur_label+'0'] = [setG__, setH__]
            M[v]=w
            search(future_)
            del M[v]
        setG_ = setG.difference([v])
        del future[selected_label]
        if len(setG_) > 0:
            future[selected_label] = [setG_, setH]
        search(future)
    
    search(future)
    return incumbent


# # CTindex

# In[4]:



def create_dimas_graph_format(graph, id_map, embedding):
    inverted_id_map = {idx:node for node,idx in id_map.items()}
    lines = ['p edge {} {} {}\n'.format(len(graph.nodes), len(graph.edges), embedding.shape[-1])]
    lines.append(' '.join(map(str, embedding)) + '\n')
    for n in range(len(inverted_id_map)):
        label = graph.nodes[inverted_id_map[n]]['label']
        lines.append('n {} {}\n'.format(n, label))
    for u, v in graph.edges():
        lines.append('e {} {}\n'.format(id_map[u], id_map[v]))
    return lines


# # Graph Embedding

# In[5]:

N_GRAPHS = 1000
dataset = sys.argv[1]
pickle_dir = '{}_pickle'.format(dataset)
if not os.path.isdir(pickle_dir):
    os.mkdir(pickle_dir)

ori_graph_data = load_data('./dataspace/graph/{}/{}'.format(dataset, dataset), supervised=False, max_degree=5, multiclass=False, use_random_walks=False)

ori_emb_model = SupervisedGraphSage(ori_graph_data.raw_feats.shape[1], 128, ori_graph_data.num_class)
ori_emb_model = ori_emb_model.cuda()
ori_emb_model.load_state_dict(torch.load('model_pt/{}_sup_20.pt'.format(dataset)))
ori_emb_model.set_params(ori_graph_data.full_adj, ori_graph_data.deg, ori_graph_data.feats)
ori_emb_model.eval()

ori_graph_emb = F.normalize(ori_emb_model.aggregator(list(range(ori_graph_data.raw_feats.shape[0]))), dim = 1)
ori_graph_emb = ori_graph_emb.detach().cpu().numpy()


for i, node in enumerate(ori_graph_data.G.nodes):
    ori_graph_data.G.nodes[node]['label'] = ori_graph_data.multi2single_label[tuple(ori_graph_data.G.nodes[node]['label'])]

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


graph_embeddings = []
ggsx_feats = []
ct_feats = []
graphs = []
# if len(os.listdir(pickle_dir)) < N_GRAPHS:
if True:
    GRAPH_SIZE = 15
    nodes = list(range(len(query_machine.ori_graph)))
    random.shuffle(nodes)

    graph_id = 0
    for core_node in tqdm(nodes,desc='gen data'):
        biggraph = query_machine.create_subgraph_from_core(core_node, GRAPH_SIZE)
        if len(biggraph) < GRAPH_SIZE or not nx.is_connected(biggraph):
            continue
        sub_id_map, sub_raw_feats, all_sub_adj, sub_degree = query_machine.create_subgraph_map(biggraph)

        embedding_biggraph = query_machine.embedding_subgraph(sub_raw_feats, all_sub_adj, sub_degree)
        embedding_biggraph = embedding_biggraph.detach().cpu().numpy().mean(0)
        with open('temp.graph', 'w') as f:
            for line in create_dimas_graph_format(biggraph, sub_id_map, embedding_biggraph):
                f.write(line)
        p = Popen(['timeout', '2', '../index/ctindex/emb', 'temp.graph', '.', '.', '-c'], stdin=PIPE, stdout=PIPE, stderr=PIPE)
        output, err = p.communicate()
        if len(err) > 0 or len(output) ==0 :
            continue
        ct_feat = np.zeros(128)
        ct_feat[list(set(map(int, output.strip().split())))] = 1

        ggsx_f = ggsx_feat(biggraph)
        # with open(os.path.join(pickle_dir, str(graph_id)+'.pkl'), 'wb') as f:
        #     pickle.dump({'graph':biggraph, 
        #                 'ctindex': ct_feat,
        #                 'ggsx': ggsx_f,
        #                 'emb': embedding_biggraph}, f)

        ggsx_f = ggsx_feat(biggraph)
        graphs.append(biggraph)
        graph_embeddings.append(embedding_biggraph)
        ggsx_feats.append(ggsx_f)
        ct_feats.append(ct_feat)

        graph_id+=1
        if graph_id >= N_GRAPHS: 
            break
else:
    for i in tqdm(range(N_GRAPHS), desc='load data'):
        data = pickle.load(open(os.path.join(pickle_dir, str(i)+'.pkl'), 'rb'))
        graphs.append(data['graph'])
        graph_embeddings.append(data['emb'])
        ggsx_feats.append(data['ggsx'])
        ct_feats.append(data['ctindex'])

graph_embeddings = np.stack(graph_embeddings)
ct_feats = np.stack(ct_feats)
ggsx_sizes = np.array([len(i) for i in ggsx_feats])
print("GGSX Size stat: ", ggsx_sizes.mean(), ggsx_sizes.std(), ggsx_sizes.max(), ggsx_sizes.min())

# In[107]:
emb_ranks, ct_ranks_nofilter, ct_ranks_filter, ggsx_ranks_nofilter, ggsx_ranks_filter = [], [], [], [], []
for _ in tqdm(range(N_GRAPHS), desc='query'):
    graph_id = random.randint(0, N_GRAPHS-1)
    subgraph = remove_node(graphs[graph_id],2)
    if not nx.is_connected(subgraph):
        continue
    sub_id_map, sub_raw_feats, all_sub_adj, sub_degree = query_machine.create_subgraph_map(subgraph)

    embedding_subgraph = query_machine.embedding_subgraph(sub_raw_feats, all_sub_adj, sub_degree)
    embedding_subgraph = embedding_subgraph.detach().cpu().numpy().mean(0)

    emb_rank = np.argwhere(np.argsort(((graph_embeddings - embedding_subgraph)**2).sum(1)) == graph_id)[0][0]
    emb_ranks.append(emb_rank)

    with open('temp.graph', 'w') as f:
        for line in create_dimas_graph_format(subgraph, sub_id_map, embedding_subgraph):
            f.write(line)
    p = Popen(['timeout', '2', '../index/ctindex/emb', 'temp.graph', '.', '.', '-c'], stdin=PIPE, stdout=PIPE, stderr=PIPE)
    output, err = p.communicate()
    if len(err) > 0 or len(output) ==0 :
        continue
    ct_feat = np.zeros(128)
    ct_feat[list(set(map(int, output.strip().split())))] = 1
    ct_rank_nofilter = np.argwhere(np.argsort(((ct_feats - ct_feat)**2).sum(1)) == graph_id)[0][0]
    ct_ranks_nofilter.append(ct_rank_nofilter)

    filter_ids, sims = [], []
    for i in range(N_GRAPHS):
        if np.all(ct_feat<= ct_feats[i]):
            filter_ids.append(i)
            sims.append((ct_feats[i]-ct_feat).sum())
    filter_ids, sims = np.array(filter_ids), np.array(sims)
    if  np.all(ct_feat<= ct_feats[graph_id]):
        ct_rank_filter = np.argwhere( filter_ids[np.argsort(sims)] == graph_id)[0][0]
        ct_ranks_filter.append(ct_rank_filter)

    ggsx_f = ggsx_feat(subgraph)
    filter_ids, sims_filter, sims_nofilter = [], [], []
    for i in range(N_GRAPHS):
        ggsx_f2 = ggsx_feats[i]
        if len(set(ggsx_f).intersection(ggsx_f2)) == len(ggsx_f):
            accept = True
            sim = 0
            for k, v in ggsx_f.items():
                if ggsx_f2[k] < v:
                    accept = False
                    break 
                sim += ggsx_f2[k] - v
            if accept:
                filter_ids.append(i)
                sims_filter.append(sim)
        sim = 0
        for k in set(ggsx_f).union(ggsx_f2):
            sim += abs(ggsx_f.get(k,0) - ggsx_f2.get(k,0))
        sims_nofilter.append(sim)

    filter_ids, sims_filter, sims_nofilter = np.array(filter_ids), np.array(sims_filter),  np.array(sims_nofilter)
    
    ggsx_rank_nofilter = np.argwhere(np.argsort(sims_nofilter) == graph_id)[0][0]
    ggsx_ranks_nofilter.append(ggsx_rank_nofilter)
    ggsx_rank_filter = np.argwhere( filter_ids[np.argsort(sims_filter)] == graph_id)[0][0]
    ggsx_ranks_filter.append(ggsx_rank_filter)
    
print(np.array(emb_ranks).mean(), \
    np.array(ct_ranks_filter).mean(), \
    np.array(ct_ranks_nofilter).mean(), \
    np.array(ggsx_ranks_filter).mean(),\
    np.array(ggsx_ranks_nofilter).mean())
