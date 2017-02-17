echo "a"
for i in $(seq 100)
do
	./bc.e ../../instancias/mw/10Reqs/minWen_10_a_full.txt 10 30 50 $i
	./elementarM2.e ../../instancias/mw/10Reqs/minWen_10_a_full.txt 10 30 50 $i
	echo "---"
done

echo "b"
for i in $(seq 100)
do      
        ./bc.e ../../instancias/mw/10Reqs/minWen_10_b_full.txt 10 30 50 $i
        ./elementarM2.e ../../instancias/mw/10Reqs/minWen_10_b_full.txt 10 30 50 $i
        echo "---"
done

echo "c"
for i in $(seq 100)
do      
        ./bc.e ../../instancias/mw/10Reqs/minWen_10_c_full.txt 10 30 50 $i
        ./elementarM2.e ../../instancias/mw/10Reqs/minWen_10_c_full.txt 10 30 50 $i
        echo "---"
done

echo "d"
for i in $(seq 100)
do      
        ./bc.e ../../instancias/mw/10Reqs/minWen_10_d_full.txt 10 30 50 $i
        ./elementarM2.e ../../instancias/mw/10Reqs/minWen_10_d_full.txt 10 30 50 $i
        echo "---"
done

echo "e"
for i in $(seq 100)
do      
        ./bc.e ../../instancias/mw/10Reqs/minWen_10_e_full.txt 10 30 50 $i
        ./elementarM2.e ../../instancias/mw/10Reqs/minWen_10_e_full.txt 10 30 50 $i
        echo "---"
done
