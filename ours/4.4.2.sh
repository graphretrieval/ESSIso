for dataset in yeast human cora citeseer
do
    for size in 100
    do
        for ((i=0;i<=19;i++))
        do
            ./out -t 0.1 -e 100 -c -s $size -r ../../final_exp/data/${dataset}_10_10/d.dimas ../../final_exp/data/${dataset}_10_10/query$i/ ../../final_exp/data/${dataset}_15_coldstart/query/ | tee results/${dataset}_${i}_coldstart.txt
            ./out -t 0.1 -e 100 -c -s $size -r ../../final_exp/data/${dataset}_10_10/d.dimas ../../final_exp/data/${dataset}_10_10/query$i/ | tee results/${dataset}_${i}.txt
        done
    done 
    echo "======================"
done