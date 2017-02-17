reset
set encoding iso_8859_1
set output './graspTTT.eps'
set terminal postscript eps enhanced

set xlabel "CPU times to optimal solution"
set xr [0:12]
set xtics 0,2,12
set ylabel "Probability"
set yr [0:1]
set ytics 0,0.2,1
set size 0.7
set key bottom right
plot "grasp-com-PR-ee.dat" using ($1):($2) title "path-relinking: ON" pt 2, "grasp-sem-PR-ee.dat" using ($1):($2) title "path-relinking: OFF" pt 8
