tout=0.1
for graph_size in 25 20 15 10
do
    for dataset in yeast human cora citeseer pubmed
    do
        datadir="../data/${dataset}_nnode${graph_size}_fam50_rate10_case20_len1000_randomFalse"
        savedir="../results/${dataset}_nnode${graph_size}"
        if [ ! -d $savedir ]
        then
            mkdir -p $savedir
        fi

        for ((i=0;i<=9;i++))
        do
            resultfile="${savedir}/exp7_mqo_${dataset}_nnode${graph_size}_fam50_rate10_case20_len1000_randomFalse_set${i}_tout${tout}.txt"
            if [ -f $resultfile ]
            then
               continue
            fi
            echo ${dataset}_${graph_size}_${i}
            ../MQO/test/a.out -dg ${datadir}/d.graph -qg ${datadir}/q${i}.graph -subIso TURBOISO -queryProTest -mqo -maxrc 5000 -nresult 100 -out temp -tout  ${tout} -dim 128 -batch 50 > ${savedir}/exp7_mqo_${dataset}_nnode${graph_size}_fam50_rate10_case20_len1000_randomFalse_set${i}_tout${tout}.txt
        done
    done
done
