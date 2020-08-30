for size in 10
do
	for dataset in wordnet
	do
		for ((i=0;i<=19;i++))
		do
			for ((p=0;p<=49;p++))
			do
				resultfile="results/tout0.1_${dataset}_${size}_10_${i}_part${p}_overhead.txt"
				notsucceed=1
				if [ -f $resultfile ]
				then
					test=`tail -n 2 $resultfile`
					if [[ $test == " MQO Time:"* ]]
					then
						notsucceed=0
						echo "Succeed $dataset $i"
					fi
				fi
				./a.out -dg ../../final_exp/data/${dataset}_${size}_10/d.graph -qg ../../final_exp/data/${dataset}_${size}_10/q$i.$p.graph -subIso TURBOISO -queryProTest -mqo -maxrc -1 -nresult 100  -out temp.txt -tout 0.1 -batch 20 >  $resultfile
		
				test=`tail -n 2 $resultfile`
				if [[ $test == " MQO Time:"* ]]
				then
					notsucceed=0
					echo "Succeed $dataset $i"
				else
					rm $resultfile
					echo "Fail $dataset $i"
				fi
			done
			echo "=================================="
		done
	done
done


