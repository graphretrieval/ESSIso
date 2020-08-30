for dataset in $1
do
    for dim in 4 8 16 32 64 128 512
    do
        rm ./dim_results/${dataset}_10_10_dim${dim}.txt
    done 
done

for dataset in $1
do
    for dim in 4 8 16 32 64 128 512
    do
        for ((i=0;i<=19;i++))
        do
            echo "./out -t 0.1 -e 100 -c -s 20 -d  ../../final_exp/temp_data/${dataset}_10_10_dim${dim}/d.dimas ../../final_exp/temp_data/${dataset}_10_10_dim${dim}/query${i}/"
            ./out -t 0.1 -e 100 -c -s 20 -d  ../../final_exp/temp_data/${dataset}_10_10_dim${dim}/d.dimas ../../final_exp/temp_data/${dataset}_10_10_dim${dim}/query${i}/ | grep "Hit:" | grep -Eo "[0-9]*" >> ./dim_results/${dataset}_10_10_dim${dim}.txt
        done 
    done 
done

for dataset in $1
do
    for dim in 4 8 16 32 64 128 512
    do
        echo ./dim_results/${dataset}_10_10_dim${dim}.txt
        python3 ./dim_results/infer.py ./dim_results/${dataset}_10_10_dim${dim}.txt
    done 
done