import sys
a=[]
for line in open(sys.argv[1]):
    a.append(float(line.strip().split()[-1]))
b=sum(a)/len(a)
print(b/1000)

