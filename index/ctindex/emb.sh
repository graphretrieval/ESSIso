for dataset in yeast human cora citeseer pubmed wordnet
do
    for size in 10 15 20 25 30
    do
        for ((i=0;i<=19;i++))
        do
            echo ${dataset}_${size}_${i}
            for filename in $(seq -f "../../../final_exp/data/${dataset}_${size}_10/query$i/q%04g.dimas" 0 999)
            do   
                timeout 2 ./emb $filename ../../../query_emb/W_${dataset}.weight ../../../query_emb/b_${dataset}.weight -e >> results/${dataset}_${size}_emb.txt
            done
        done
        #python3 infer_emb_time.py emb_time/${dataset}_emb_time_${size}_e.txt
    done
done
