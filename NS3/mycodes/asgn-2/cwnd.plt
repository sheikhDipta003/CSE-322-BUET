set terminal png
set output ARG1
set title "Congestion Window vs Time"
set xlabel "Time (s)"
set ylabel "Cwnd (bytes)"
plot ARG2 using int(ARG6):int(ARG7) with linespoints title ARG4, ARG3 using int(ARG6):int(ARG7) with linespoints title ARG5