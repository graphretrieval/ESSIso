import numpy as np
import sys
data = []
timeout = 0
for line in open(sys.argv[1]):
    line = line.strip().split()
    if len(line)==0: timeout+=1
    else: data.append(int(line[-1]))
data = np.array(data)
print(data.mean())
print(data.std())
print(timeout)
