for c in 10 20 30 40 50
do 
    for dataset in yeast human cora citeseer
    do 
        for ((i=0;i<=19;i++))
        do 
            echo $i >> ../results/${dataset}_${c}.txt
            ./out -t 0.1 -e 100 -c -s $c -h -k 1 -d -b ../data/test_edge/yeast_nnode10_fam50_rate10_case20_len1000_randomFalse/d.dimas ../data/test_edge/yeast_nnode10_fam50_rate10_case20_len1000_randomFalse/query${i}_weighted/ | tail -n 3 >> ../results/${dataset}_${c}.txt
        done
    done
done