for dataset in yeast human cora citeseer pubmed wordnet
do
    python3 create_stream_coldstart_kmean.py --prefix dataspace/graph/${dataset}/${dataset}  --epochs 20 --pretrained_model  model_pt/${dataset}_sup_20.pt --save_model  model_pt/${dataset}_sup_20.pt  --cuda --num_subgraphs 20 --embedding_model ../data --rate 100
done
