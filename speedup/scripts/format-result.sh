#!/bin/bash

echo "Formatting results..."

datasets=$1
raw_perf_results_dir=$2
formated_perf_results_dir=$3
user=$4
iterations=$5
max_j=$6

# features="branch-instructions branch-misses bus-cycles cache-misses cache-references cpu-cycles instructions ref-cycles L1-dcache-load-misses L1-dcache-loads L1-dcache-stores L1-icache-load-misses LLC-load-misses LLC-loads LLC-store-misses LLC-stores branch-load-misses branch-loads dTLB-load-misses dTLB-loads dTLB-store-misses dTLB-stores iTLB-load-misses iTLB-loads node-load-misses node-loads node-store-misses node-stores explanatory-power-maximization-time"

for dataset in $datasets
do
	mkdir $formated_perf_results_dir/$dataset
	for ((j=1; j<=$max_j; j++))
	do
		mkdir $formated_perf_results_dir/$dataset/j$j
	done
done

for ((iteration=1; iteration<=$iterations; iteration++))
do
	for dataset in $datasets
	do
		for ((j=1; j<=$max_j; j++))
		do
			input_file=$raw_perf_results_dir/$iteration/$dataset-result-j$j.txt
			output_file=$formated_perf_results_dir/$dataset/j$j/$iteration.txt

			# Use awk to extract the relevant data and format it
			awk '/^[[:space:]]*[0-9]+/ && !/seconds/ {print $1, $2}' $input_file > $output_file
			awk '/explanatory-power-maximization-time[[:space:]]*/ {print $2, $1}' $input_file >> $output_file
			# Delete last line
			# head -n -1 $output_file > temp.txt && mv temp.txt $output_file
			
			# 	sed 's/\([0-9]\+,[0-9]\+\) +-/\1 total_time/' $output_file > $output_file
			# chown $user:$user $output_file
		done
	done
done
