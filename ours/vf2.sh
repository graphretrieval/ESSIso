tout=1.0
for graph_size in 20 25 30
do 
    for dataset in yeast human cora citeseer pubmed wordnet
    do 
        datadir="../data/${dataset}_nnode${graph_size}_fam50_rate10_case20_len1000_randomFalse"
        savedir="../results/${dataset}_nnode${graph_size}"
        if [ ! -d savedir ] 
        then
            mkdir -p $savedir
        fi

        for ((i=0;i<=9;i++))
        do 
            echo ${dataset}_${graph_size}_${i}
            ./out -t ${tout} -e 100 -v ${datadir}/d.dimas ${datadir}/query${i}_weighted/  > ${savedir}/exp7_vf2_${dataset}_nnode${graph_size}_fam50_rate10_case20_len1000_randomFalse_set${i}_tout${tout}.txt
        done
    done
done
