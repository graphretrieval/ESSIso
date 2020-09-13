for t in 1.0 0.75 0.5 0.25 
do 
    for dataset in pubmed wordnet
    do 
        for ((i=0;i<=9;i++))
        do 
            echo ${dataset}_${t}_${i}_False
            for ((j=0;j<=1;j++))
            do
                ./out -t $t -e 100 -c -s 50 -h -k 1 -d -b ../data/${dataset}_nnode10_fam50_rate10_case20_len1000_randomFalse/d.dimas ../data/${dataset}_nnode10_fam50_rate10_case20_len1000_randomFalse/query${i}_weighted/  >> ../results/tout/ours_${dataset}_${t}_${i}_${j}_randomFalse.txt
                ./out -t $t -e 100 -c -s 50 -h -k 1 -b ../data/${dataset}_nnode10_fam50_rate10_case20_len1000_randomFalse/d.dimas ../data/${dataset}_nnode10_fam50_rate10_case20_len1000_randomFalse/query${i}_weighted/ >> ../results/tout/recache_${dataset}_${t}_${i}_${j}_randomFalse.txt
                ./out -t $t -e 100 -c -s 50 -h -k 1 -r -b ../data/${dataset}_nnode10_fam50_rate10_case20_len1000_randomFalse/d.dimas ../data/${dataset}_nnode10_fam50_rate10_case20_len1000_randomFalse/query${i}_weighted/ >> ../results/tout/lru_${dataset}_${t}_${i}_${j}_randomFalse.txt
            done
        done
    done
done

#for t in 0.1 0.25 0.5 0.75 1.0
#do 
#    for dataset in yeast human cora citeseer
#    do 
#        for ((i=0;i<=9;i++))
#        do 
#            echo ${dataset}_${t}_${i}_False
#            for ((j=0;j<=1;j++))
#            do
#                ./out -t $t -e 100 -c -s 50 -h -k 1 -d -b ../data/${dataset}_nnode10_fam50_rate10_case20_len1000_randomTrue/d.dimas ../data/${dataset}_nnode10_fam50_rate10_case20_len1000_randomTrue/query${i}_weighted/  >> ../results/tout/ours_${dataset}_${t}_${i}_${j}_randomTrue.txt
#                ./out -t $t -e 100 -c -s 50 -h -k 1 -b ../data/${dataset}_nnode10_fam50_rate10_case20_len1000_randomTrue/d.dimas ../data/${dataset}_nnode10_fam50_rate10_case20_len1000_randomTrue/query${i}_weighted/ >> ../results/tout/recache_${dataset}_${t}_${i}_${j}_randomTrue.txt
#                ./out -t $t -e 100 -c -s 50 -h -k 1 -r -b ../data/${dataset}_nnode10_fam50_rate10_case20_len1000_randomTrue/d.dimas ../data/${dataset}_nnode10_fam50_rate10_case20_len1000_randomTrue/query${i}_weighted/ >> ../results/tout/lru_${dataset}_${t}_${i}_${j}_randomTrue.txt
#            done
#        done
#    done
#done
