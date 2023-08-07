#!/bin/bash

mkdir -p ./scratch/1905003/plots
gnu_file=./scratch/1905003/results.gnuplot

plot_all()
{
    # args [starting from 1, AFTER $gnu_file] :
    # 1 -> output-file-name, 2 -> plot-title, 3 -> xlabel, 4 -> ylabel, 5 -> path-to-the-data-file, 6 -> x-data-col, 7 -> y-data-col
    gnuplot -c $gnu_file "$1/throughput_$2.png" "throughput vs $2" "$2" "throughput (Kbps)" "$3" "$4" 5
    gnuplot -c $gnu_file "$1/delivery_ratio_$2.png" "delivery ratio vs $2" "$2" "delivery ratio" "$3" "$4" 6
}


# vary no of nodes
file=./scratch/1905003/plots/data_nodes.txt
rm -f $file
touch $file

for i in $(seq 20 20 100) #inclusive
do
    ./ns3 run "scratch/1905003/1905003_1.cc --nNodes=${i} --nFlows=${i} --file=${file}"
    echo "Node : $i done"
done

plot_all "./scratch/1905003/plots" "nodes" $file 1


# vary no of flows
file=./scratch/1905003/plots/data_flow.txt
rm -f $file
touch $file

for i in $(seq 10 10 50) #inclusive
do
    ./ns3 run "scratch/1905003/1905003_1.cc --nFlows=${i} --file=${file}"
    echo "Flow : $i done"
done

plot_all "./scratch/1905003/plots" "flow" $file 2


# vary no of packets per second
file=./scratch/1905003/plots/data_pps.txt
rm -f $file
touch $file

for i in $(seq 100 100 500) #inclusive
do
    ./ns3 run "scratch/1905003/1905003_1.cc --nPacketsPerSecond=${i} --file=${file}"
    echo "Packets per s : $i done"
done

plot_all "./scratch/1905003/plots" "packets per second" $file 3


# vary coverage area
file=./scratch/1905003/plots/data_coverage.txt
rm -f $file
touch $file

for i in $(seq 1 1 5) #inclusive
do
    ./ns3 run "scratch/1905003/1905003_1.cc --coverageArea=${i} --file=${file}"
    echo "Coverage Area : $i done"
done

plot_all "./scratch/1905003/plots" "coverage area" $file 4