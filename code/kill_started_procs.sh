#!/bin/bash

sleep_time=0.5

directory=$(pwd)

tmp_pid_file=".tmp_pids"

num_of_procs=$(wc -l < "${tmp_pid_file}")

echo "${num_of_procs}"

loop_signal_at_process_func(){
	limit="${3}"

	for (( i=0; i<limit; i++ ));
	do
		pkill -"$2" -P "$1"
	done
}


while read -r the_pid
do
	echo "Trying to kill process of pid = $the_pid ..."
	loop_signal_at_process_func "${the_pid}" 2 3
	loop_signal_at_process_func "${the_pid}" 9 1
	echo "Attempt made!"
	sleep "${sleep_time}"

done < "${tmp_pid_file}"

cat /dev/null > "${tmp_pid_file}"
