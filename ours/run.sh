tout=1.0
for graph_size in 10
do 
    for dataset in yeast human cora citeseer pubmed worndet
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
            ./out -t ${tout} -e 100 -c -s 50 -h -k 1 -d -b ${datadir}/d.dimas ${datadir}/query${i}_weighted/  | tee ${savedir}/exp7_ours_${dataset}_nnode${graph_size}_fam50_rate10_case20_len1000_randomFalse_set${i}_tout${tout}.txt
            ./out -t ${tout} -e 100 ${datadir}/d.dimas ${datadir}/query${i}_weighted/  | tee ${savedir}/exp7_turboiso_${dataset}_nnode${graph_size}_fam50_rate10_case20_len1000_randomFalse_set${i}_tout${tout}.txt
            ./out -t ${tout} -e 100 -v ${datadir}/d.dimas ${datadir}/query${i}_weighted/  | tee ${savedir}/exp7_vf2_${dataset}_nnode${graph_size}_fam50_rate10_case20_len1000_randomFalse_set${i}_tout${tout}.txt
            ../MQO/test/a.out -dg ${datadir}/d.graph -qg ${datadir}/q${i}.graph -subIso TURBOISO -queryProTest -mqo -maxrc 5000 -nresult 100 -out temp -tout  ${tout} -dim 128 -batch 50 | tee ${savedir}/exp7_mqo_${dataset}_nnode${graph_size}_fam50_rate10_case20_len1000_randomFalse_set${i}_tout${tout}.txt
        done
    done
done
