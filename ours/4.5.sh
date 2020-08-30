for gsize in 10 15
do
    for dataset in yeast human cora citeseer
    do
        for ((i=0;i<=19;i++))
        do
            # echo "${dataset}_${gsize}_vf2"
            # ./out -t 1 -e 100 -v ../../final_exp/data/${dataset}_${gsize}_10/d.dimas ../../final_exp/data/${dataset}_${gsize}_10/query2/ | grep "Total running time:"
            echo "${dataset}_${gsize}_turboiso"
            ./out -t 1 -e 100 ../../final_exp/data/${dataset}_${gsize}_10/d.dimas ../../final_exp/data/${dataset}_${gsize}_10/query$i/ | grep "Total running time:"
            echo "${dataset}_${gsize}_ours"
            ./out -t 1 -e 100 -c -s 20 -d ../../final_exp/data/${dataset}_${gsize}_10/d.dimas ../../final_exp/data/${dataset}_${gsize}_10/query$i/ | grep "Total running time:"
        done
    done
    echo "======================"
done