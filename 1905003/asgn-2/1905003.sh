#!/bin/bash

gnuFile1="1905003/cwnd.plt"
gnuFile2="1905003/thrput.plt"
gnuFile3="1905003/fairnessIndex.plt"

rm -rf 1905003 || rmdir 1905003

# choose tcp conjestion control protocol and plottitle2
tcp="ns3::TcpHighSpeed"; plottitle2="tcphs";
[ "$1" == 1 ] && { tcp="ns3::TcpHighSpeed"; plottitle2="tcphs"; }
[ "$1" == 2 ] && { tcp="ns3::TcpAdaptiveReno"; plottitle2="tcpar"; }
[ "$1" == 3 ] && { tcp="ns3::TcpWestwoodPlus"; plottitle2="tcpwwp"; }

mkdir -p "1905003/$plottitle2"
path="1905003/$plottitle2"
bttlnkRates=("1" "5" "10" "20" "50" "100" "200" "300")

# vary bttlnkRate 
for i in "${bttlnkRates[@]}"; do
    ./ns3 run "1905003/1905003.cc --tcp2=$tcp --bttlnkRate=$i --results=$path --experimentNo=1"

    gnuplot -c $gnuFile1 "$path/cwnd_bttlnkRate_${i}.png" "${path}/flow1.dat" "${path}/flow2.dat" "newreno" "$plottitle2" 1 2
    echo "bttlnkRate $i done"
done

gnuplot -c $gnuFile2 "$path/throughput_bttlnkRate.png" "Throughput VS Bottleneck Link Capacity" "Bottleneck Link Capacity (Mbps)" "$path/data_1.txt" "$plottitle2" 1 3 4
gnuplot -c $gnuFile3 "$path/fairnessIndex_bttlnkRate.png" "Fairness Index VS Bottleneck Link Capacity" "Bottleneck Link Capacity (Mbps)" "Fairness Index" "$path/data_1.txt" 1 5

# vary pcktLossExp
for i in {2..6}; do
    ./ns3 run "1905003/1905003.cc --tcp2=$tcp --pcktLossExp=$i --results=$path --experimentNo=2"

    gnuplot -c $gnuFile1 "$path/cwnd_loss_rate_${i}.png" "${path}/flow1.dat" "${path}/flow2.dat" "newreno" "$plottitle2" 1 2
    echo "loss rate 1e-$i done"
done

gnuplot -c $gnuFile2 "$path/throughput_pcktLossRate.png" "Throughput VS Packet Loss Rate" "Packet Loss Rate Exponent" "$path/data_2.txt" "$plottitle2" 2 3 4
gnuplot -c $gnuFile3 "$path/fairnessIndex_pcktLossRate.png" "Fairness Index VS Packet Loss Rate" "Packet Loss Rate Exponent" "Fairness Index" "$path/data_2.txt" 2 5
