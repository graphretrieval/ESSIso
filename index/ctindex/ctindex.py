from subprocess import Popen, PIPE
from collections import defaultdict

from scipy.stats import spearmanr, pearsonr
from tqdm import tqdm 
import sys 

dataset = sys.argv[1]
mcs2ct1 = defaultdict(list)
mcs2ct2 = defaultdict(list)
for i in tqdm(range(1000)):
    p = Popen(['timeout', '10','./emb', '../../../final_exp/ctindex_data/{}/q{}.graph'.format(dataset, i), '.', '.', '-c'], stdin=PIPE, stdout=PIPE, stderr=PIPE)
    output, err = p.communicate()
    if len(err) > 0 or len(output) ==0 :
        continue
    embedding_biggraph = set(map(int, output.strip().split()))
    for dist in range(1,8):
        p = Popen(['timeout', '10','./emb', '../../../final_exp/ctindex_data/{}/q{}.{}.graph'.format(dataset, i, dist), '.', '.', '-c'], stdin=PIPE, stdout=PIPE, stderr=PIPE)
        output, err = p.communicate()
        if len(err) > 0 or len(output) ==0 :
            continue
        embedding_subgraph = set(map(int, output.strip().split()))
        # mcs2ct1[dist].append(len(embedding_biggraph.intersection(embedding_subgraph)))
        mcs2ct2[dist].append(len(embedding_biggraph.symmetric_difference(embedding_subgraph)))

# X = []
# Y = []
# for k,v in mcs2ct1.items():
#     X += [k] * len(v)
#     Y += v
# print(spearmanr(X, Y), pearsonr(X,Y))
X = []
Y = []
for k,v in mcs2ct2.items():
    X += [k] * len(v)
    Y += v
print(spearmanr(X, Y), pearsonr(X,Y))
print(len(X))
    

