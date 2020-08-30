
for dataset in yeast human cora citeseer
do
    ./a.out -dg ../../../final_exp/data/${dataset}_20_10/d.graph -qg ../../../final_exp/data/${dataset}_20_10/q$1.graph -subIso TURBOISO -queryProTest -sqp -maxrc 100 -nresult 10  -out temp.txt -tout 100 | grep "SQO Average Call:"
    ./a.out -dg ../../../final_exp/data/${dataset}_20_10/d.graph -qg ../../../final_exp/data/${dataset}_20_10/q$1.graph -subIso TURBOISO -queryProTest -sqp -maxrc 100 -nresult 10  -out temp.txt -tout 100 -node | grep "SQO Average Call:"
    echo "======================"
done
