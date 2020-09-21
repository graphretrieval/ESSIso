tout=0.1
graph_size=10
fam=50
for w in 1
do
    for dataset in yeast human cora citeseer pubmed wordnet
    do
        for rate in 10 20 30 40 50 60 70 80 90
        do
            datadir="../data/rate_exp_new/${dataset}_nnode${graph_size}_fam${fam}_rate${rate}_case50_len1000_randomFalse"
            savedir="../results/rate_exp_new/${dataset}_nnode${graph_size}_fam${fam}"
            if [ ! -d $savedir ]
            then
                mkdir -p $savedir
            fi

            for ((i=0;i<=9;i++))
            do
                echo ${dataset}_${rate}_${w}_${i}
                ./out -t ${tout} -e 100 -c -s 50 -h -k 1 -d -b -w ${w} ${datadir}/d.dimas ${datadir}/query${i}_weighted/  > ${savedir}/exp7_ours_${dataset}_nnode${graph_size}_fam${fam}_rate${rate}_case20_len1000_randomFalse_set${i}_tout${tout}_w${W}.txt
            done
        done
    done
done

# for rate in 10 20 30 40 50 60 70 80 90
# do
#     for dataset in yeast human cora citeseer pubmed wordnet
#     do
#         datadir="../data/rate_exp_new/${dataset}_nnode${graph_size}_fam${fam}_rate${rate}_case50_len1000_randomFalse"
#         savedir="../results/rate_exp_new/${dataset}_nnode${graph_size}_fam${fam}"
#         if [ ! -d $savedir ]
#         then
#             mkdir -p $savedir
#         fi

#         for ((i=0;i<=9;i++))
#         do
#             echo ${dataset}_${rate}_${i}
#             ./out -t ${tout} -e 100 -c -s 50 -h -k 1 -d -b ${datadir}/d.dimas ${datadir}/query${i}_weighted/  > ${savedir}/exp7_ours_${dataset}_nnode${graph_size}_fam${fam}_rate${rate}_case20_len1000_randomFalse_set${i}_tout${tout}.txt
#         done
#     done
# done

for rate in 10 20 30 40 50 60 70 80 90
do
    for dataset in wordnet
    do
        datadir="../data/rate_exp/${dataset}_nnode10_fam${fam}_rate${rate}_case20_len1000_randomFalse"
        savedir="../results/rate_exp/${dataset}_nnode10_fam${fam}"
        if [ ! -d $savedir ]
        then
            mkdir -p $savedir
        fi

        for ((i=0;i<=9;i++))
        do
            echo ${dataset}_${rate}_${i}
            ./out -t ${tout} -e 100 -c -s 50 -h -k 1 -d -b ${datadir}/d.dimas ${datadir}/query${i}_weighted/  > ${savedir}/exp7_ours_${dataset}_nnode10_fam${fam}_rate${rate}_case20_len1000_randomFalse_set${i}_tout${tout}.txt
        done
    done
done
