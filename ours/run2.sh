for dataset in yeast human cora citeseer
do 
    for ((i=0;i<=9;i++))
    do 
        echo ${dataset}_${i}
        for ((j=0;j<=1;j++))
        do
            ./out -t 0.1 -e 100 ../data/${dataset}_nnode10_fam50_rate10_case20_len1000_randomFalse/d.dimas ../data/${dataset}_nnode10_fam50_rate10_case20_len1000_randomFalse/query${i}_weighted/  >> ../results/nocache_${dataset}_${i}_${j}_randomFalse.txt
        done
    done
done
