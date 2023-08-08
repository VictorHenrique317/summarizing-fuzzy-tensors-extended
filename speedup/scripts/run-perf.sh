#!/bin/bash

echo "	Running nclusterbox..."

raw_perf_results_dir=$1
user=$2
datasets=$3
iteration=$4
max_j=$5

mkdir $raw_perf_results_dir/$iteration
# chown $user:$user $raw_perf_results_dir/$iteration

for dataset in $datasets
do
	echo "		Dataset: $dataset"
	for ((j=1; j<=$max_j; j++))
		do
		echo "		===> -j=$j"

		output_file=$raw_perf_results_dir/$iteration/$dataset-result-j$j.txt
		additional_params=""

		if [[ $dataset == "school" ]]; then
			additional_params="--b"
		fi

		
		nclusterbox -v2 -m1000 $additional_params -j$j datasets/$dataset -o /dev/null > temp1-log.txt
		
		explanatory_time=$(grep "Explanatory power maximization time:" temp1-log.txt | awk '{print $5}' | tr -d 's')
		rm temp1-log.txt
		echo "explanatory-power-maximization-time    "$explanatory_time >> $output_file

		# chown $user:$user $output_file
	done
done
