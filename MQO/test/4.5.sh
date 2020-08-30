for gsize in 10 15 20 25 30
do
    for dataset in yeast human cora citeseer
    do
        ./a.out -dg ../../../final_exp/data/${dataset}_${gsize}_10/d.graph -qg ../../../final_exp/data/${dataset}_${gsize}_10/q0.graph -subIso TURBOISO -queryProTest -mqo -maxrc -1 -nresult 100  -out temp.txt -tout 1 -batch 20 | grep "MQO Time:"
    done
    echo "======================"
done