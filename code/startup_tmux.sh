#!/data/data/com.termux/files/usr/bin/bash

sleep_time=2

sleep_time_mult_attach_cmd_proc=3

curr_term_started=2

session_name="my_stuff"

termux_tmp_pid_file=".termux_tmp_pids"


directory="$(pwd)"



start_another_proc_func_inner(){

	echo "$BASHPID" >> $termux_tmp_pid_file
	tmux new-window -t session_name:$curr_term_started "$1"
	curr_term_started=$(($curr_term_started+1))

}

start_another_proc_func(){

	sleep $sleep_time

	start_another_proc_func_inner "$1"&

}

pushd $directory

cat /dev/null >|  $termux_tmp_pid_file

tmux new-session -d -s session_name

start_another_proc_func "bash -lc 'cd \"./port_mapper\" && ./port_mapper.exe; exec bash'"

start_another_proc_func "bash -lc 'cd \"./master_server\" && ./master_server.exe; exec bash'"

start_another_proc_func "bash -lc 'cd \"./server\" && ./server.exe; exec bash'"

start_another_proc_func "bash -lc 'cd \"./client\"; exec bash'"

start_another_proc_func "bash -lc 'cd \"./server_browser\"; exec bash'"

start_another_proc_func "bash -lc 'bash ./edit_configs.sh; exec bash'"

start_another_proc_func "bash -lc 'cd \"./converter_tool\" ; exec bash'"

cat $tmp_pid_file

sleep $(($sleep_time*$sleep_time_mult_attach_cmd_proc)) && tmux attach -t my_stuff


popd
