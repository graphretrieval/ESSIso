from __future__ import division
from __future__ import print_function

import torch
import torch.nn.functional as F
from graphsage.dataset import Dataset 
from graphsage.model import UnsupervisedGraphSage, SupervisedGraphSage
from graphsage.run import train_model
from utils.query_machine import GraphQuery, normalize
from scipy.spatial.distance import cdist
from tqdm import tqdm
import networkx as nx
import time
import argparse
import numpy as np
import json
import os
import pdb
import random
from collections import Counter 

#from graphsage-simple.model import run_graph
def load_data(prefix, supervised=False, max_degree=25, multiclass=False, use_random_walks=True, load_walks=True, num_walk=50, walk_len=5):
    dataset = Dataset()
    dataset.load_data(prefix=prefix, normalize=True, supervised=False, max_degree=max_degree, multiclass=multiclass, use_random_walks = use_random_walks, load_walks=load_walks, num_walk=num_walk, walk_len=walk_len, train_all_edge=True)
    return dataset

def parse_args():
    parser = argparse.ArgumentParser(description="Query embedding")
    parser.add_argument('--embedding_model', default="unsup/graphsage_mean_0.001000/")
    parser.add_argument('--seed', default= 42, type = int)
    parser.add_argument('--prefix', default= "dataspace/graph/ppi/graphsage/ppi")
    parser.add_argument('--batch_size', default = 512  , type = int)
    parser.add_argument('--identity_dim', default = 0,  type = int)
    parser.add_argument('--learning_rate', default = 0.001, type = float)
    parser.add_argument('--neg_sample_size', default = 2, type = int)
    parser.add_argument('--max_degree', default = 5, type = int)
    parser.add_argument('--cuda', action = "store_true")
    parser.add_argument('--dim', default = 128, type = int)
    parser.add_argument('--unsup', action='store_true')
    parser.add_argument('--dir', default = './dataspace/graph/')
    parser.add_argument('--random_delete_nodes', default = 0.0, type = float)

    parser.add_argument('--multiclass', action='store_true', help='Whether use 1-hot labels or indices.')
    parser.add_argument('--epochs', default=1, type=int, help='Number of epochs to train.')
    parser.add_argument('--max_subgraph_nodes', default=10, type=int, help='whether to use features')
    parser.add_argument('--num_subgraphs', default=10, type=int, help='number of testing subgraphs')
    parser.add_argument('--pretrained_model', default='', help='trained model')
    parser.add_argument('--save_model', default='model.pt', help='trained model')
    parser.add_argument('--num_candidates', default=10, type=int, help='number of candidates for each query nodes')
    parser.add_argument('--num_new_nodes', default=0, type=int, help='number of nodes to be added')
    parser.add_argument('--distance', default='mean', help='mean, sum or all')
    parser.add_argument('--num_remove', type=int, default=1, help='Number of edges or nodes to be removed')
    parser.add_argument('--rate', type=int, default=10, help='Number of edges or nodes to be removed')
    return parser.parse_args()
    
def get_model_and_data(args):

    ori_graph_data = load_data(args.prefix, supervised=False, max_degree=args.max_degree, multiclass=False, use_random_walks=False)
    print(Counter(np.squeeze(ori_graph_data.true_labels).tolist()))
    a = dict(Counter(np.squeeze(ori_graph_data.true_labels).tolist()))
    label_dict = np.array([i for i in a.values()])
    print(label_dict.mean(), label_dict.std())
    print("Feature shape: {}".format(ori_graph_data.raw_feats.shape))

    if args.unsup:
        ori_emb_model = UnsupervisedGraphSage(ori_graph_data.raw_feats.shape[1], args.dim)
    else:
        ori_emb_model = SupervisedGraphSage(ori_graph_data.raw_feats.shape[1], args.dim, ori_graph_data.num_class)
    
    if args.cuda:
        ori_emb_model.cuda()
    
    if os.path.isfile(args.pretrained_model):
        print("load from pretrained model...")
        ori_emb_model.load_state_dict(torch.load(args.pretrained_model))
        ori_emb_model.set_params(ori_graph_data.full_adj, ori_graph_data.deg, ori_graph_data.feats)
    else:
        print("training from scratch...")
        train_model(ori_emb_model, ori_graph_data, args.batch_size, args.epochs, args.learning_rate, args.save_model)
        torch.save(ori_emb_model.state_dict(), args.save_model)

    ori_emb_model.eval()
    ori_graph_emb = F.normalize(ori_emb_model.aggregator(list(range(ori_graph_data.raw_feats.shape[0]))), dim = 1)
    ori_graph_emb = ori_graph_emb.detach().cpu().numpy()

    return ori_emb_model, ori_graph_emb, ori_graph_data

if __name__ == '__main__':
    start_time = time.time()
    args = parse_args()
    print(args)

    random.seed(args.seed)
    np.random.seed(args.seed)
    torch.manual_seed(args.seed)
    torch.cuda.manual_seed(args.seed)

    ori_emb_model, ori_graph_emb, ori_graph_data = get_model_and_data(args)
  
    query_machine = GraphQuery(ori_emb_model, 
                            ori_graph_emb,
                            ori_graph_data.G,
                            ori_graph_data.id_map,
                            ori_graph_data.feats,
                            ori_graph_data.raw_feats,
                            ori_graph_data.full_adj,
                            ori_graph_data.deg)

    if args.num_new_nodes > 0:
        n_origin_nodes = len(query_machine.ori_graph.nodes)
        ori_graph_emb = query_machine.add_new_nodes(args.num_new_nodes) 

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
    import sys
    sys.exit(0)
    import os
    if not os.path.isfile('new_ori_graph_emb_yeast.npy'):
        new_ori_graph_emb = np.zeros_like(ori_graph_emb)
        for i in tqdm(ori_graph_data.G.nodes()):
            subgraph = ori_graph_data.G.subgraph(stat(i))
            embedding_subgraph, sub_id_map = get_embedding_subgraph(subgraph)
            new_ori_graph_emb[i] = embedding_subgraph[sub_id_map[i]]
        np.save('new_ori_graph_emb_yeast.npy', new_ori_graph_emb)
    else:
        new_ori_graph_emb = np.load('new_ori_graph_emb_yeast.npy')
    centroid_accs = []
    times = []
    times2 = []
    graph_accs = []
    graph_accs2 = []

    for _ in tqdm(range(args.num_subgraphs),desc='evalutaion'):
        # create subgraph
        subgraph = query_machine.create_subgraph_from_core(random.choice(range(len(query_machine.ori_graph.nodes))), args.max_subgraph_nodes)
        if len(subgraph.nodes())==1: continue
        sub_id_map, sub_raw_feats, all_sub_adj, sub_degree = query_machine.create_subgraph_map(subgraph)

        # embedding subgraph
        embedding_subgraph = query_machine.embedding_subgraph(sub_raw_feats, all_sub_adj, sub_degree)
        embedding_subgraph = embedding_subgraph.detach().cpu().numpy()

        graph_acc2, running_time2 = query_machine.exact_query_graph(subgraph, sub_id_map)
        print(graph_acc2, running_time2)

        # graph_acc, running_time = query_machine.query_graph(normalize(ori_graph_emb), normalize(embedding_subgraph), subgraph, sub_id_map, args.num_candidates)
        graph_acc, running_time = query_machine.query_graph(new_ori_graph_emb, embedding_subgraph, subgraph, sub_id_map, args.num_candidates)
        print(graph_acc, running_time)

        # centroid_acc = query_machine.query_nodes(normalize(ori_graph_emb), normalize(embedding_subgraph), subgraph, sub_id_map)
        centroid_acc = query_machine.query_nodes(new_ori_graph_emb, embedding_subgraph, subgraph, sub_id_map)

        graph_accs.append(graph_acc)
        graph_accs2.append(graph_acc2)
        times.append(running_time)
        times2.append(running_time2)
        centroid_accs.append(centroid_acc)

    graph_accs = np.mean(np.array(graph_accs))
    graph_accs2 = np.mean(np.array(graph_accs2))
    times = np.mean(np.array(times))
    times2 = np.mean(np.array(times2))
    print(max(centroid_accs), min(centroid_accs))
    centroid_accs = (np.mean(np.array(centroid_accs)), np.std(np.array(centroid_accs)))

    print("Centroid ranking: {}".format(centroid_accs))
    print("Graph acc: {}, Time : {}".format(graph_accs, times))
    print("Graph acc2: {}, Time2 : {}".format(graph_accs2, times2))

   

