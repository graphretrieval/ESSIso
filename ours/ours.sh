tout=0.5
graph_size=10
for fam in 10 20 30 40 50 60 70 80 90 100
do
    for dataset in wordnet
    do
        datadir="../data/wordnet_fam/${dataset}_nnode${graph_size}_fam${fam}_rate10_case20_len1000_randomFalse"
        savedir="../results/fam_exp/${dataset}_nnode${graph_size}_fam${fam}"
        if [ ! -d $savedir ]
        then
            mkdir -p $savedir
        fi

        for ((i=0;i<=9;i++))
        do
            echo ${dataset}_${fam}_${i}
            ./out -t ${tout} -e 100 -c -s 50 -h -k 1 -d -b ${datadir}/d.dimas ${datadir}/query${i}_weighted/  > ${savedir}/exp7_ours_${dataset}_nnode${graph_size}_fam${fam}_rate10_case20_len1000_randomFalse_set${i}_tout${tout}.txt
        done
    done
done
