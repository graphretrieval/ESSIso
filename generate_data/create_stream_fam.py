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
from collections import defaultdict

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
# import sys
# sys.exit(0)
avg_deg = [ori_graph_data.G.degree(i) for i in ori_graph_data.G.nodes]
print(len(ori_graph_data.G.nodes), len(ori_graph_data.G.edges))
print(len(set(ori_graph_data.multi2single_label.values())))
print(len(list(ori_graph_data.multi2single_label.keys())[0]))
print(sum(avg_deg)/len(avg_deg))
print(Counter([ori_graph_data.multi2single_label[tuple(ori_graph_data.G.nodes[i]['label'])] for i in ori_graph_data.G]))
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
# print(new_ori_graph_emb.shape)
new_ori_graph_emb = ori_graph_emb

first_value = int(args.rate)
x = int((100-first_value)/3)
second_value = first_value+x 
thirs_value = second_value+x 


# n_family = args.num_subgraphs
RANDOM = False
n_family = 50
N_CASES = 20
LENGTH = 1000
MAX_FAMILY = 100
for max_subgraph_nodes in [10]:
    for rate in [10]:
    # for rate in [args.rate]:
        for n_family in range(10, 101, 10):
            savedir = os.path.join(args.embedding_model, '{}_nnode{}_fam{}_rate{}_case{}_len{}_random{}'.format(args.prefix.split('/')[-1], max_subgraph_nodes, n_family, rate, N_CASES, LENGTH, RANDOM))
            if not os.path.isdir(savedir):
                os.makedirs(savedir)
            # else:
            #     continue

            with open(os.path.join(savedir, 'd.dimas'), 'w') as f:
                for line in create_dimas_graph_format(ori_graph_data.G, ori_graph_data.id_map, new_ori_graph_emb, 0, ori_graph_data.multi2single_label):
                    f.write(line)

            with open(os.path.join(savedir, 'd.graph'), 'w') as f:
                for line in create_graph_format(ori_graph_data.G, ori_graph_data.id_map, new_ori_graph_emb, 0, ori_graph_data.multi2single_label):
                    f.write(line)

        for set_index in tqdm(range(10)):
            # core_nodes = random.sample([node for node in query_machine.ori_graph], min(3000, len(query_machine.ori_graph)))
            # core_nodes = list(query_machine.ori_graph.nodes)
            labels_dict = defaultdict(list)
            for node in ori_graph_data.G.nodes:
                if ori_graph_data.multi2single_label[ tuple(ori_graph_data.G.nodes[node]['label']) ] == 2:
                    labels_dict[0].append(node)
                else:
                    labels_dict[1].append(node)

            # random.shuffle(core_nodes)
            for i in labels_dict:
                random.shuffle(labels_dict[i])

            core_graphs = []
            case_dict = {}
            # for core_node in core_nodes:
            curr_label = 0
            curr_nodes = labels_dict[curr_label]
            while(True):
                core_node = curr_nodes[0]
                # if ori_graph_data.multi2single_label[ tuple(ori_graph_data.G.nodes[core_node]['label']) ] == 2:
                #     continue
                n_tries = 0
                success = False
                while(True):
                    sampled_node = stat_(core_node, max_subgraph_nodes)
                    biggraph = ori_graph_data.G.subgraph(sampled_node)
                    n_tries+=1
                    if nx.is_connected(biggraph) and len(biggraph) == max_subgraph_nodes:
                        core_graphs.append(biggraph)
                        success = True
                        break
                    if n_tries > 10:
                        break  
                del curr_nodes[0] 
                if success:
                    curr_label = 1 - curr_label 
                    curr_nodes = labels_dict[curr_label]
                    case_dict[len(core_graphs)-1] = {'case1': biggraph, 'case2':[], 'case3':[]}
                    # N_CASES = random.randint(10,100)
                    for _ in range(N_CASES):
                        n_tries = 0
                        while(True):
                            new_sampled_node = [i for i in biggraph.nodes()]
                            n_more_node = random.randint(1,2)
                            # n_more_node = 1
                            candidates_node = set()
                            for node in new_sampled_node:
                                for neigh_node in ori_graph_data.G.neighbors(node):
                                    if neigh_node not in new_sampled_node:
                                        candidates_node.add(neigh_node)
                            new_sampled_node += random.sample(candidates_node, min(len(candidates_node), n_more_node))
                            subgraph = ori_graph_data.G.subgraph(new_sampled_node)
                            if len(subgraph)  < max_subgraph_nodes:
                                break
                            if nx.is_connected(subgraph):
                                case_dict[len(core_graphs)-1]['case2'].append(subgraph)
                                break
                            n_tries+=1
                            if n_tries >=20:
                                break
                        n_tries = 0
                        while(True):
                            n_more_node = random.randint(1,2)
                            subgraph = remove_node(biggraph, n_more_node)
                            if len(subgraph)  < max_subgraph_nodes-1:
                                break
                            if nx.is_connected(subgraph):
                                case_dict[len(core_graphs)-1]['case3'].append(subgraph)
                                break
                            n_tries+=1
                            if n_tries >=20:
                                break
                if len(core_graphs) >= MAX_FAMILY:
                    break
            for n_family in range(10, 101, 10):
                savedir = os.path.join(args.embedding_model, '{}_nnode{}_fam{}_rate{}_case{}_len{}_random{}'.format(args.prefix.split('/')[-1], max_subgraph_nodes, n_family, rate, N_CASES, LENGTH, RANDOM))
                subgraph_queries = core_graphs[:int(n_family)]
                family_indexes = list(range(int(n_family)))

                print(len(core_graphs))
                while(len(subgraph_queries) < LENGTH):
                    random_index = random.choice(family_indexes[-int(n_family):])
                    # random_index = random.randint(0, n_family-1)
                    random_number = random.randint(1,100)
                    if 1 <= random_number and random_number <= rate:
                        subgraph_queries.append(case_dict[random_index]['case1'])
                        family_indexes.append(random_index)
                    elif random_number < 2*rate:
                        subgraph_queries.append(random.choice(case_dict[random_index]['case2']))
                        family_indexes.append(random_index)
                    elif random_number < 3*rate:
                        subgraph_queries.append(random.choice(case_dict[random_index]['case3']))
                        family_indexes.append(random_index)
                    else:
                        temp_list = set(range(n_family))
                        temp_list = temp_list.difference(family_indexes[-int(n_family):])
                        temp_list = list(temp_list)
                        if len(temp_list) ==0:
                            temp_list = list(range(n_family))
                            temp_list.remove(random_index)
                        random_index = random.choice(temp_list)
                        subgraph_queries.append(case_dict[random_index]['case1'])
                        family_indexes.append(random_index)
                print(family_indexes)

                query_lines = [] 
                dimas_query_lines = [] 
                dimas_query_lines_weighted = [] 
                if RANDOM:
                    c = list(zip(subgraph_queries, family_indexes))
                    random.shuffle(c)

                    subgraph_queries, family_indexes = zip(*c)

                for i, subgraph in tqdm(enumerate(subgraph_queries), desc="gen embedding subgraph"):
                    sub_id_map, sub_raw_feats, all_sub_adj, sub_degree = query_machine.create_subgraph_map(subgraph)

                    embedding_subgraph = query_machine.embedding_subgraph(sub_raw_feats, all_sub_adj, sub_degree)
                    embedding_subgraph = embedding_subgraph.detach().cpu().numpy()
                    if not np.isnan(embedding_subgraph).any():
                        dimas_query_lines.append(create_dimas_graph_format(subgraph, sub_id_map, embedding_subgraph, i, ori_graph_data.multi2single_label, family_indexes[i]))
                        dimas_query_lines_weighted.append(create_dimas_graph_format(subgraph, sub_id_map, embedding_subgraph, i, ori_graph_data.multi2single_label, family_indexes[i], True))
                        query_lines += create_graph_format(subgraph, sub_id_map, embedding_subgraph, i, ori_graph_data.multi2single_label)

                if not os.path.isdir(os.path.join(savedir, 'query{}'.format(set_index))):
                    os.mkdir(os.path.join(savedir, 'query{}'.format(set_index)))
                if not os.path.isdir(os.path.join(savedir, 'query{}_weighted'.format(set_index))):
                    os.mkdir(os.path.join(savedir, 'query{}_weighted'.format(set_index)))
                for i, query_line in enumerate(dimas_query_lines):
                    with open(os.path.join(savedir,  'query{}'.format(set_index) , 'q{:04d}.dimas'.format(i)), 'w') as f:
                        for line in query_line:
                            f.write(line)
                for i, query_line in enumerate(dimas_query_lines_weighted):
                    with open(os.path.join(savedir,  'query{}_weighted'.format(set_index) , 'q{:04d}.dimas'.format(i)), 'w') as f:
                        for line in query_line:
                            f.write(line)
                with open(os.path.join(savedir, 'q{}.graph'.format(set_index)), 'w') as f:
                    for line in query_lines:
                        f.write(line)