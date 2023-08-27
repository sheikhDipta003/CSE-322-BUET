set terminal png
set output ARG1
set title ARG2
set xlabel ARG3
set ylabel "Throughput (Kbps)"
plot ARG4 using int(ARG6):int(ARG7)  with linespoints title "newreno", ARG4 using int(ARG6):int(ARG8) with linespoints title ARG5