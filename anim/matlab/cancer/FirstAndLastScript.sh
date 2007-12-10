#!/bin/sh
#
# This script looks through all the 40minute and 9 hour files generated by
# CHASTE TestMeinekeLabellingExperiment.hpp
#
# It compiles the output from several different runs, fill in the SETUP 
# section below.

# BEGIN SETUP
results=/local/pmxgm/Simulation_Results/16_stem_cell_Simple_Wnt/sunter3
run[0]="2007-11-23-15-05"
run[1]="2007-11-23-15-08"
run[2]="2007-11-23-15-12"
run[3]="2007-11-23-15-17"
run[4]="2007-11-23-15-20"
first_experiment=300
last_experiment=800
# END OF SETUP

if cd $results; then
	rm -f first_lines.txt
	rm -f last_lines.txt
else
	echo "Could not change directory! Aborting." 1>&2
	exit 1
fi

for ((j = 0 ; j<${#run[*]} ; j++))	# Number of program runs
do 
	echo "Compiling results from run $j"
	for (( i=$first_experiment ; i<=$last_experiment ; i=i+10 ))
	do
		head -1 ${run[$j]}/MeinekeLabellingExperiment/results_from_time_"$i".667/vis_results/results.viznodes >> first_lines.txt
		tail -1 ${run[$j]}/MeinekeLabellingExperiment/results_from_time_"$i".667/vis_results/results.viznodes >> last_lines.txt
	done
done