for dataset in yeast human cora citeseer
do
    for size in 10 15 20
    do
        echo "${dataset}_${size}_lru"
        ./out -t 0.1 -e 100 -c -s $size -r ../../final_exp/data/${dataset}_10_10/d.dimas ../../final_exp/data/${dataset}_10_10/query2/ | grep "Hit"
        echo "${dataset}_${size}_recache"
        ./out -t 0.1 -e 100 -c -s $size ../../final_exp/data/${dataset}_10_10/d.dimas ../../final_exp/data/${dataset}_10_10/query2/ | grep "Hit"
        echo "${dataset}_${size}_ours"
        ./out -t 0.1 -e 100 -c -s $size -d ../../final_exp/data/${dataset}_10_10/d.dimas ../../final_exp/data/${dataset}_10_10/query2/ | grep "Hit"
    done 
    echo "======================"
done