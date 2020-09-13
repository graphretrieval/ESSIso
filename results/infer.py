import numpy as np
import sys
import os

dataset = sys.argv[1]
size = sys.argv[2]

datadir = '{}_nnode{}'.format(dataset, size)

for method in ['vf2', 'turboiso', 'mqo', 'ours']:
    times = []
    founds = []
    for i in range(10):
        filename = os.path.join(datadir, 'exp7_{}_{}_nnode{}_fam50_rate10_case20_len1000_randomFalse_set{}_tout0.1.txt'.format(method, dataset, size, i))
        if not os.path.isfile(filename):
            print('File not exist')
            continue

        if method == 'mqo':
            lines = open(filename).readlines()
            if len(lines)<2 or 'MQO Time' not in lines[-2]:
                print(filename, 'err')
                # os.remove(filename)
                continue
            run_time = float(lines[-2].split()[-1].split('(')[0])
            found = int(lines[-1].split()[-1])
        else:
            lines = open(filename).readlines()
            if len(lines)<3 or 'Total running time' not in lines[-3]:
                print(filename, 'err')
                continue
            run_time = float(lines[-3].split()[-2])
            found = int(lines[-2].split()[-2])

        times.append(run_time)
        founds.append(found)
    print(method, np.array(times).mean()/1000, np.array(founds).mean())
