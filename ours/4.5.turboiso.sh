for gsize in 10 15
do
    for dataset in yeast human cora citeseer
    do
        for ((i=0;i<=19;i++))
        do
            echo "${dataset}_${gsize}_turboiso"
            ./out -t 1 -e 100 ../../final_exp/data/${dataset}_${gsize}_10/d.dimas ../../final_exp/data/${dataset}_${gsize}_10/query$i/ | grep "Total running time:" >> turboiso.txt
        done
    done
    echo "======================"
done