#!/bin/bash


our_cwd="$(pwd)/"

module_log_directory_name="/tmux_logs/"

directories_to_work_on=("port_mapper" "content_server" "heartbeat_server" "client" "server_browser" "master_server")

complete_directories_to_work_on=()

complete_log_directories_to_work_on=()

echo "$our_cwd"

total=${#directories_to_work_on[@]}

echo ${total}

print_directories(){
	for((i=0; i<$total; i++)); 
	do
		echo "$our_cwd${directories_to_work_on[$i]}"
	done
}

fill_complete_directories(){
	for ((i=0; i<total; i++));
	do
		complete_directories_to_work_on[i]="${our_cwd}${directories_to_work_on[$i]}"
	done
}

fill_complete_log_directories(){
	for ((i=0; i<total; i++));
	do
		complete_log_directories_to_work_on[i]="${complete_directories_to_work_on[$i]}$module_log_directory_name"
	done
}

print_log_directories(){
	for((i=0; i<$total; i++)); 
	do
		echo "${complete_log_directories_to_work_on[$i]}"
	done
}


cre8_log_directories(){

	for((i=0; i<$total; i++));
	do
		mkdir "${complete_log_directories_to_work_on[$i]}"
	done


}

remove_log_directories(){

	for((i=0; i<$total; i++));
	do
		rm -rvf "${complete_log_directories_to_work_on[$i]}"
	done


}

fill_complete_directories

fill_complete_log_directories

print_log_directories


remove_log_directories
