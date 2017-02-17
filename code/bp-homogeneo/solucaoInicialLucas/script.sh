for n in 10 15 20 25 30
do
	if [ $n -eq 10 ]
	then 
		k=4
	fi
	if [ $n -eq 15 ]
	then 
		k=6
	fi
	if [ $n -eq 20 ]
	then 
		k=7
	fi
	if [ $n -eq 25 ]
	then 
		k=9
	fi
	if [ $n -eq 30 ]
	then 
		k=10
	fi
	for inst in a b c d e
	do
		for ((  i = 0 ;  i <= 100;  i++  ))
		do
			echo "./geraColsHomo.e ../../../instancias/mwCluster/$n Reqs-k$k/minWen_$n _$inst.txt $n $k 20 >> si_minWen_$n _$inst.txt"
		done
	done
done
