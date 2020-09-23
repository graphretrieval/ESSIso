import os
import sys
import numpy as np
datadir = sys.argv[1]

running_times_mqo = []
running_times_ours = []
overhead_times_mqo = []
overhead_times_ours = []
for filename in [os.path.join(datadir, i) for i in os.listdir(datadir)]:
    if 'mqo' in filename:
        for line in open(filename):
            if 'Overhead' in line:
                overhead_times_mqo.append(float(line.strip().split()[-1])/1000)
            elif 'MQO' in line:
                running_times_mqo.append( float( line.strip().split()[-1].split('(')[0] )/1000 )
    elif 'ours' in filename:
        for line in open(filename):
            if 'overhead' in line:
                overhead_times_ours.append( float( line.strip().split()[-2] )/1000)
            elif 'running' in line:
                running_times_ours.append( float( line.strip().split()[-2] )/1000)
    else:
        print('wtf') 
print(np.array(running_times_mqo).mean())
print(np.array(overhead_times_mqo).mean())
print(np.array(running_times_ours).mean())
print(np.array(overhead_times_ours).mean())
