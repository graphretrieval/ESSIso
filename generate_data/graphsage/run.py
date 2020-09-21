import torch
import numpy as np
from graphsage.model import UnsupervisedGraphSage, SupervisedGraphSage

"""
Simple supervised GraphSAGE model as well as examples running the model
on the Cora and Pubmed datasets.
"""

def train_model(model, graph_data, batch_size, epochs, learning_rate, save_model):
    model.set_params(graph_data.full_adj, graph_data.deg, graph_data.feats)

    optimizer = torch.optim.Adam(filter(lambda p : p.requires_grad, model.parameters()), lr=learning_rate)
    is_cuda = next(model.parameters()).is_cuda
    is_unsup = isinstance(model, UnsupervisedGraphSage)
    if is_unsup:
        train_data = graph_data.all_edges
    else:
        train_data = graph_data.nodes 

    n_iters = len(train_data)//batch_size
    for epoch in range(epochs):
        print("Epoch {0}".format(epoch))
        np.random.shuffle(train_data)
        for iter in range(n_iters):
            optimizer.zero_grad()
            batch_data = train_data[iter*batch_size:(iter+1)*batch_size]
            if is_unsup:
                batch_data = torch.LongTensor(batch_data)
                loss = model.loss(batch_data[:,0], batch_data[:,1])
            else:
                batch_labels = torch.LongTensor(np.squeeze(graph_data.true_labels[batch_data]))
                if is_cuda:
                    batch_labels = batch_labels.cuda()

                batch_data = torch.LongTensor(batch_data)
                loss = model.loss(batch_data, batch_labels)

            loss.backward()
            optimizer.step()
            if iter % 4 == 0:
                print(loss)
        torch.save(model.state_dict(), save_model[:-3]+'_'+str(epoch)+save_model[-3:])
    return model


        
