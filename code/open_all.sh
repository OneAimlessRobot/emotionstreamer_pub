#!/bin/bash



modules=("server" "client" "master_server" "heartbeat" "port_mapper" "server_browser" "converter_tool")


length=${#modules[@]}

curr_dir=$(pwd)

commands=("bash openSources.sh" "bash openHeaders.sh" "pwd" "pwd")
number_of_windows=( 4 4 4 4 4 4 4 )

echo $curr_dir

echo $length

for ((i=0;i<length;i++)); do
modules[i]="$curr_dir/${modules[i]}";
done

for ((i=0;i<length;i++)); do
echo ${modules[i]};
done


for ((i=0;i<length;i++)); do
	cd ${modules[i]}
	command="xterm"
	for ((j=0;j<${#commands[@]};j++)); do
		$command&
	done
done
