#!/bin/bash
gnu_file1="scratch/asgn-2/plotB_cwnd.plt"
gnu_file2="scratch/asgn-2/plotB_data.plt"
gnu_file3="scratch/asgn-2/plotA.plt"

rm -rf taskB || rmdir taskB
mkdir -p taskB

prep_files()
{
    mkdir -p taskB/"$1"
    touch taskB/"$1"/data_1.txt
    touch taskB/"$1"/data_2.txt
}

# first arg selects method, second argument selects no of nodes
if [ "$1" == 1 ]; then
    tcp="ns3::TcpHighSpeed"
    method="tcphs"
elif [ "$1" == 2 ]; then
    tcp="ns3::TcpAdaptiveReno"
    method="areno"
else [ "$1" == 3 ]
    tcp="ns3::TcpWestwoodPlus"
    method="westwoodplus"
fi
prep_files $method
path="taskB"/"$method"

# task 
# for i in $(seq 50 10 100) #inclusive
arr=("1" "5" "10" "20" "100" "300")
for i in "${arr[@]}";
do
    ./ns3 run "scratch/asgn-2/task-1.cc --tcp2=${tcp} --bttlnkRate=${i} --output_folder=${path} --exp=1" >> "${path}/${method}.txt" 2>&1
    # output file name | file 1 | file1_1stcol | file1_2ndcol | title 1 | file2 | file2_2ndcol | title 2
    gnuplot -c $gnu_file1 "$path/cwnd_bttlnkRate_${i}.png" "${path}/flow1.cwnd" 1 2 "newreno" "${path}/flow2.cwnd" 2 ${method}
    echo "bttlnkRate $i done"
done

# output file name | title | x label | file 1 | file1_1stcol | file1_2ndcol | file1_3rdcol | title 2
gnuplot -c $gnu_file2 "$path/throughput vs bttlnkRate.png" "Throughput VS Bottleneck Link Capacity" "Bottleneck Link Capacity (Mbps)" "$path/data_1.txt" 1 3 4 ${method}
gnuplot -c $gnu_file3 "$path/fairness index vs bttlnkRate.png" "Fairness Index VS Bottleneck Link Capacity" "Bottleneck Link Capacity (Mbps)" "Fairness Index" "$path/data_1.txt" 1 5 "newreno + ${method}"

# # # task 
for i in {2..6}
do
    ./ns3 run "scratch/asgn-2/task-1.cc --tcp2=${tcp} --plossRate=${i} --output_folder=${path} --exp=2">> "${path}/${method}.txt" 2>&1
    # output file name | file 1 | file1_1stcol | file1_2ndcol | title 1 | file2 | file2_2ndcol | title 2
    gnuplot -c $gnu_file1 "$path/cwnd_loss_rate_${i}.png" "${path}/flow1.cwnd" 1 2 "newreno" "${path}/flow2.cwnd" 2 ${method}
    echo "loss rate exp -$i done"
done


# output file name | title | x label | file 1 | file1_1stcol | file1_2ndcol | file1_3rdcol | title 2
gnuplot -c $gnu_file2 "$path/throughput vs packet loss rate.png" "Throughput VS Packet Loss Rate" "Packet Loss Rate (%) Exponent" "$path/data_2.txt" 2 3 4 ${method}
gnuplot -c $gnu_file3 "$path/fairness index vs packet loss rate.png" "Fairness Index VS Packet Loss Rate" "Packet Loss Rate (%) Exponent" "Fairness Index" "$path/data_2.txt" 2 5 "newreno + ${method}"