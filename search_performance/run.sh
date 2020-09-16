for dataset in yeast human cora citeseer pubmed wordnet
do
    for size in 10 15 20 25 30
    do
        echo "${dataset}_${size}"
        ./out -t 0.001 -k 1 ../data/search_performance/${dataset}_nnode${size}_ndata50_case10_search_performance/data_weighted/ ../data/search_performance/${dataset}_nnode${size}_ndata50_case10_search_performance/query_weighted/ | tail -n 2
    done 
done