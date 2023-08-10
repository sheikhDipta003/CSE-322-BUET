#!/bin/bash

rm -rf ./scratch/assignment/part_2_wireless_high_mobile/plots
mkdir -p ./scratch/assignment/part_2_wireless_high_mobile/plots
gnu_file=./scratch/assignment/results.gnuplot

plot_all()
{
    # args [starting from 1, AFTER $gnu_file] :
    # 1 -> output-file-name, 2 -> plot-title, 3 -> xlabel, 4 -> ylabel, 5 -> path-to-the-data-file, 6 -> x-data-col, 7 -> y-data-col
    gnuplot -c $gnu_file "$1/throughput_$2.png" "throughput vs $2" "$2" "throughput (Kbps)" "$3" "$4" 5
    gnuplot -c $gnu_file "$1/delivery_ratio_$2.png" "delivery ratio vs $2" "$2" "delivery ratio" "$3" "$4" 6
}


# vary no of nodes
file=./scratch/assignment/part_2_wireless_high_mobile/plots/data_nodes.txt
rm -f $file
touch $file

for i in $(seq 20 20 100) #inclusive
do
    ./ns3 run "scratch/assignment/part_2_wireless_high_mobile/wireless_high_mobile.cc --nNodes=${i} --nFlows=$((i/2)) --file=${file}"
    echo "Node : $i done"
done

plot_all "./scratch/assignment/part_2_wireless_high_mobile/plots" "nodes" $file 1


# vary no of flows
file=./scratch/assignment/part_2_wireless_high_mobile/plots/data_flow.txt
rm -f $file
touch $file

for i in $(seq 10 10 50) #inclusive
do
    ./ns3 run "scratch/assignment/part_2_wireless_high_mobile/wireless_high_mobile.cc --nFlows=${i} --file=${file}"
    echo "Flow : $i done"
done

plot_all "./scratch/assignment/part_2_wireless_high_mobile/plots" "flow" $file 2


# vary no of packets per second
file=./scratch/assignment/part_2_wireless_high_mobile/plots/data_pps.txt
rm -f $file
touch $file

for i in $(seq 100 100 500) #inclusive
do
    ./ns3 run "scratch/assignment/part_2_wireless_high_mobile/wireless_high_mobile.cc --nPacketsPerSecond=${i} --file=${file}"
    echo "Packets per s : $i done"
done

plot_all "./scratch/assignment/part_2_wireless_high_mobile/plots" "packets per second" $file 3


# vary speed
file=./scratch/assignment/part_2_wireless_high_mobile/plots/data_speed.txt
rm -f $file
touch $file

for i in $(seq 5 5 25) #inclusive
do
    ./ns3 run "scratch/assignment/part_2_wireless_high_mobile/wireless_high_mobile.cc --speed=${i} --file=${file}"
    echo "Speed : $i done"
done

plot_all "./scratch/assignment/part_2_wireless_high_mobile/plots" "speed" $file 4