method=recache
 for size in 10
 do
     for cache in 20
     do
         for dataset in wordnet
         do
             for rate in 10
             do
                 for t in 0.2 0.25 0.5 0.75 1
                 do
                     resultdir="../../final_results/tout${t}_cache${cache}_${dataset}_${size}_${rate}_new"
                     datadir="../data/${dataset}_${size}_${rate}_new"
                     if [ ! -d $resultdir ]
                     then
                         mkdir $resultdir
                     fi
 
                     for ((i=0;i<=2;i++))
                     do
                         resultfile="$resultdir/${method}$i.txt"
                         notsucceed=1
                         if [ -f $resultfile ]
                         then
                             test=`tail -n 1 $resultfile`
                             if [[ $test == "Number of found:"* ]]
                             then
                                 notsucceed=0
                                 echo "Succeed $i"
                             fi
                         fi
                         tries=0
                         while [ $notsucceed -eq 1 ]
                         do
                             ./out -e 100 -s $cache -t $t -c ${datadir}/d.dimas ${datadir}/query$i/ > $resultfile
                             test=`tail -n 1 $resultfile`
                             ((tries++))
                             if [ $tries -ge 10 ]
                             then
                                 notsucceed=0
                                 echo "Give up $i"
                                 rm $resultfile
                             elif [[ $test == "Number of found:"* ]]
                             then
                                 notsucceed=0
                                 echo "Succeed $i"
                             else
                                 echo "Re-run $i"
                             fi
                         done
                         echo "=================================="
                     done
                 done
             done
         done
     done
 done
