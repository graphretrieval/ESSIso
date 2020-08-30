import numpy as np 
import sys 

data = [int(line.strip()) for line in open(sys.argv[1])]
print(np.array(data).mean())