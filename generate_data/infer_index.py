#!/usr/bin/env python
# coding: utf-8

# In[1]:


import torch
import random 
import numpy as np 
from tqdm import tqdm 
import networkx as nx
import numpy as np
from scipy.stats import pearsonr, spearmanr
from collections import defaultdict
import time
from subprocess import Popen, PIPE
import pickle
import sys
import signal
import os
# # GGSX
random.seed(0)

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

N_GRAPHS = 200
dataset = sys.argv[1]
pickle_dir = '{}_pickle'.format(dataset)

graph_embeddings = []
ggsx_feats = []
ct_feats = []
graphs = []
for i in tqdm(random.sample(list(range(1000)), 200), desc='load data'):
    data = pickle.load(open(os.path.join(pickle_dir, str(i)+'.pkl'), 'rb'))
    graphs.append(data['graph'])
    graph_embeddings.append(data['emb'])
    ggsx_feats.append(data['ggsx'])
    ct_feats.append(data['ctindex'])

graph_embeddings = np.stack(graph_embeddings)
ct_feats = np.stack(ct_feats)
ggsx_sizes = np.array([len(i) for i in ggsx_feats])
print("GGSX Size stat: ", ggsx_sizes.mean(), ggsx_sizes.std(), ggsx_sizes.max(), ggsx_sizes.min())

edit_distances, emb_distances, ggsx_distances, ct_distances = [],[] ,[],[]
for _ in tqdm(range(1000)):
    i, j = random.randint(0, N_GRAPHS-1), random.randint(0, N_GRAPHS-1)
    graph1 = graphs[i]
    graph2 = graphs[j]
    edit_distance = len(mcs(graph1, graph2))
    edit_distances.append(edit_distance)

    emb_distance = np.linalg.norm(graph_embeddings[i] - graph_embeddings[j])
    emb_distances.append(emb_distance)

    ct_distance = np.linalg.norm(ct_feats[i] - ct_feats[j])
    ct_distances.append(ct_distance)

    ggsx_f1 = ggsx_feats[i]
    ggsx_f2 = ggsx_feats[j]
    ggsx_distance = 0
    for k in set(ggsx_f1).union(ggsx_f2):
        ggsx_distance += abs(ggsx_f1.get(k,0) - ggsx_f2.get(k,0))
    ggsx_distances.append(ggsx_distance)  

for name, v in {'emb_distances':emb_distances, 'ct_distances': ct_distances, 'ggsx_distances': ggsx_distances}.items():
    print (name , (spearmanr(edit_distances, v).correlation, pearsonr(edit_distances, v)[0]))
# In[107]:
