for dataset in yeast human cora citeseer
do
    for size in 10 15
    do
        for ((i=0;i<=19;i++))
        do
            for filename in $(seq -f "../../../final_exp/data/${dataset}_${size}_10/query$i/q%04g.dimas" 0 999)
            do   
                ./emb $filename ../../../query_emb/W_${dataset}.weight ../../../query_emb/b_${dataset}.weight -c >> ${dataset}_${size}_cti.txt
            done
        done
        python3 infer_emb_time.py emb_time/${dataset}_emb_time_${size}_e.txt
    done
done
