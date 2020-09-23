import numpy as np
import sys
from tqdm import tqdm
data = []
timeout = 0
for line in tqdm(open(sys.argv[1])):
    line = line.strip().split()
    data.append(int(line[-1]))
data = np.array(data)
print(data.mean())
# print(data.std())
# print(timeout)
