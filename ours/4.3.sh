for dataset in $1
    do
        for size in 20
        do
            for ((i=0;i<=19;i++))
            do
                echo "${dataset}_${size}_index"
                ./out -t 0.1 -e 100 -c -s $size -r ../../final_exp/data/${dataset}_10_10/d.dimas ../../final_exp/data/${dataset}_10_10/query$i/ | grep "Total scan time" >> ${dataset}_index.txt
                echo "${dataset}_${size}_non-index"
                ./out_wa -t 0.1 -e 100 -c -s $size -r ../../final_exp/data/${dataset}_10_10/d.dimas ../../final_exp/data/${dataset}_10_10/query$i/ | grep "Total scan time" >> ${dataset}_nonindex.txt
            done
        done 
    echo "======================"
    done
done