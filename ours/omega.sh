for dataset in $1
do 
    for size in 10 15 20
    do 
        for ((w=2;w<=10;w++))
        do
            rm ./omega_results/${dataset}_${size}_10_${w}.txt
        done
    done
done 

for dataset in $1
do 
    for size in 10 15 20
    do 
        for ((i=0;i<=19;i++))
        do 
            for ((w=2;w<=10;w++))
            do
                ./out -t 0.1 -e 100 -c -s 20 -d -h -w $w ../../final_exp/data/${dataset}_${size}_10/d.dimas ../../final_exp/data/${dataset}_${size}_10/query$i/ | grep "Total running time:" | grep -Eo "[0-9]*" >> ./omega_results/${dataset}_${size}_10_${w}.txt
            done
        done
    done
done 

for dataset in $1
do 
    for size in 10 15 20
    do 
        for ((w=2;w<=10;w++))
        do
            echo ./omega_results/${dataset}_${size}_10_${w}.txt
            python3 omega_results/infer.py ./omega_results/${dataset}_${size}_10_${w}.txt
        done
    done
done 
