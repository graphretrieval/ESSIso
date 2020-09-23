import numpy as np
import sys
import os

dataset = sys.argv[1]
size = 10
# datadir = 'fam_exp/{}_nnode{}_fam{}'.format(dataset, size, fam)
method = 'ours'
fam=50
for rate in range(10,95,10):
    datadir = 'rate_exp_new/{}_nnode{}_fam{}'.format(dataset, size, fam)
    times = []
    founds = []
    for i in range(10):
        for j in ['','_1','_2']:
            filename = os.path.join(datadir, 'exp7_{}_{}_nnode{}_fam50_rate{}_case50_len1000_randomFalse_set{}_tout0.5_w1.txt'.format(method, dataset, size, rate, i, j))
            if not os.path.isfile(filename):
                print('File not exist')
                continue

            lines = open(filename).readlines()
            if len(lines)<3 or 'Total running time' not in lines[-3]:
                print(filename, 'err')
                #os.remove(filename)
                continue
            run_time = float(lines[-3].split()[-2])
            found = int(lines[-2].split()[-2])

            times.append(run_time)
            founds.append(found)
    print(rate , np.array(times).mean()/1000, np.array(founds).mean())
