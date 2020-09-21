import numpy as np
import sys
import os

dataset = sys.argv[1]
size = 10
# datadir = 'fam_exp/{}_nnode{}_fam{}'.format(dataset, size, fam)
method = 'ours'

for fam in range(10, 101, 10):
    datadir = 'fam_exp/{}_nnode{}_fam{}'.format(dataset, size, fam)
    times = []
    founds = []
    for i in range(10):
        filename = os.path.join(datadir, 'exp7_{}_{}_nnode{}_fam{}_rate10_case20_len1000_randomFalse_set{}_tout0.5.txt'.format(method, dataset, size, fam, i))
        if not os.path.isfile(filename):
            print('File not exist')
            continue

        else:
            lines = open(filename).readlines()
            if len(lines)<3 or 'Total running time' not in lines[-3]:
                print(filename, 'err')
                continue
            run_time = float(lines[-3].split()[-2])
            found = int(lines[-2].split()[-2])

        times.append(run_time)
        founds.append(found)
    print(fam, np.array(times).mean()/1000, np.array(founds).mean())
