import torch
import torch.nn as nn
import torch.nn.functional as F
from torch.nn import init
from torch.autograd import Variable

import numpy as np
from graphsage.prediction import BipartiteEdgePredLayer
import os

"""
Simple supervised GraphSAGE model as well as examples running the model
on the Cora and Pubmed datasets.
"""
def fixed_unigram_candidate_sampler(num_sampled, unique, range_max, distortion, unigrams):
    weights = unigrams**distortion
    prob = weights/weights.sum()
    sampled = np.random.choice(range_max, num_sampled, p=prob, replace=~unique)
    return sampled

class UnsupervisedGraphSage(nn.Module):

    def __init__(self, input_dim, output_dim):
        super(UnsupervisedGraphSage, self).__init__()
        self.input_dim = input_dim
        self.output_dim = output_dim

        self.linear1 = nn.Linear(2*self.input_dim, self.input_dim)
        self.tanh = nn.Tanh()
        self.linear2 = nn.Linear(2*self.input_dim,self.output_dim)        

        self.neg_sample_size = 10
        self.normalize_embedding = True

        self.link_pred_layer = BipartiteEdgePredLayer(is_normalized_input=self.normalize_embedding)
    
    def set_params(self, adj_lists, degrees, feat_data):
        assert self.input_dim == feat_data.shape[1]

        self.adj_lists = adj_lists
        self.degrees = degrees
        
        self.feat_data = Variable(torch.FloatTensor(feat_data), requires_grad = False)
        if next(self.parameters()).is_cuda:
            self.feat_data = self.feat_data.cuda()

    def aggregator(self, nodes): 
        init_nodes = nodes
        means2 = torch.zeros(len(nodes), self.input_dim)

        for node in init_nodes:
            nodes = set(nodes).union(self.adj_lists[node])

        means1 = torch.zeros(len(nodes), self.feat_data.shape[1])
        if next(self.parameters()).is_cuda:
            means1 = means1.cuda()
            means2 = means2.cuda()

        new_id2idx = {node:i for i,node in enumerate(nodes)}
        for node in nodes:
            mean1 = self.feat_data[list(self.adj_lists[node])].mean(dim=0)
            means1[new_id2idx[node]] = mean1
        emb_hop1 = torch.cat((self.feat_data[list(nodes)], means1), dim=1)

        emb_hop1 = self.linear1(emb_hop1)
        emb_hop1 = self.tanh(emb_hop1)

        for i,node in enumerate(init_nodes):
            mean2 = emb_hop1[[new_id2idx[ele] for ele in self.adj_lists[node]]].mean(dim=0)
            means2[i] = mean2
        emb_hop2 = torch.cat((self.feat_data[list(init_nodes)], means2), dim=1)
        emb_hop = self.linear2(emb_hop2)
        return emb_hop

    def forward(self, inputs1, inputs2):        
        neg = fixed_unigram_candidate_sampler(
            num_sampled=self.neg_sample_size,
            unique=False,
            range_max=len(self.degrees),
            distortion=0.75,
            unigrams=self.degrees
        )
        outputs11 = self.aggregator(inputs1.tolist())
        outputs21 = self.aggregator(inputs2.tolist())
        neg_outputs = self.aggregator(neg.tolist())

        if self.normalize_embedding:
            outputs1 = F.normalize(outputs11, dim=1)
            outputs2 = F.normalize(outputs21, dim=1)
            neg_outputs = F.normalize(neg_outputs, dim=1)
           
        return outputs1, outputs2, neg_outputs

    def loss(self, inputs1, inputs2):
        batch_size = inputs1.size()[0]
        outputs1, outputs2, neg_outputs  = self.forward(inputs1, inputs2)        
        loss = self.link_pred_layer.loss(outputs1, outputs2, neg_outputs) / batch_size
        return loss


class SupervisedGraphSage(nn.Module):
    def __init__(self, input_dim, output_dim, n_classes):
        super(SupervisedGraphSage, self).__init__()
        self.input_dim = input_dim
        self.output_dim = output_dim

        self.linear1 = nn.Linear(2*self.input_dim, self.output_dim)
        #self.tanh = nn.Tanh()
        #self.linear2 = nn.Linear(2*self.input_dim,self.output_dim)    
        self.linear3 = nn.Linear(self.output_dim, n_classes)  
       
        self.normalize_embedding = True
        self.xent = nn.CrossEntropyLoss()
        # self.xent = nn.CrossEntropyLoss(weight=torch.tensor([1, 2846.0/4496, 2846.0/63009, 2846.0/6728, 2846.0/5501]))

    def set_params(self, adj_lists, degrees, feat_data):
        assert self.input_dim == feat_data.shape[1]

        self.adj_lists = adj_lists
        self.degrees = degrees
        
        self.feat_data = Variable(torch.FloatTensor(feat_data), requires_grad = False)
        if next(self.parameters()).is_cuda:
            self.feat_data = self.feat_data.cuda()

    def aggregator(self, nodes): 

        means = torch.zeros(len(nodes), self.feat_data.shape[1])
        if next(self.parameters()).is_cuda:
            means = means.cuda()

        new_id2idx = {node:i for i,node in enumerate(nodes)}
        for node in nodes:
            mean1 = self.feat_data[list(self.adj_lists[node])].mean(dim=0)
            means[new_id2idx[node]] = mean1

        emb_hop = torch.cat((self.feat_data[list(nodes)], means), dim=1)
        emb_hop = self.linear1(emb_hop)

        return emb_hop

    def forward(self, inputs):        
        outputs = self.aggregator(inputs.tolist())
        logits = self.linear3(F.relu(outputs))
        if self.normalize_embedding:
            logits = F.normalize(logits, dim=1)

        return logits

    def loss(self, inputs, labels):
        logits  = self.forward(inputs)        
        loss = self.xent(logits, labels)
        
        return loss
