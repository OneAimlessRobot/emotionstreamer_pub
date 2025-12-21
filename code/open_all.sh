#!/bin/bash

directory="/mnt/FASTstorage/FASTprogs/emotionstreamer/code"

#xterm -e "pushd $directory && pushd port_mapper && ./*exe && exec bash"&
#xterm -e "pushd $directory && pushd server && ./*exe  && exec bash"&
#xterm -e "pushd $directory && pushd client &&  exec bash"&



modules=("server" "client" "master_server" "heartbeat" "port_mapper" "server_browser" "converter_tool")


length=${#modules[@]}

#curr_dir=$(pwd)
curr_dir=$directory

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
	command="xterm -e \""
	for ((j=0;j<(${#commands[@]});j++)); do
		command="${command} && ${commands[j]}"
	done
	command="${command} && exec bash\"&"
	$command
done
