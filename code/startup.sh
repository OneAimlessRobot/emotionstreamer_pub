#!/bin/bash

sleep_time=0.25

directory=$(pwd)

tmp_pid_file=".tmp_pids"

term_exec_string=""

term_exec_string_case_wayland="foot -- bash -c"

term_exec_string_case_X11="xterm -e"



if [ -z "${DISPLAY}"  ];
then
	term_exec_string=$term_exec_string_case_wayland
else
	term_exec_string=$term_exec_string_case_X11
fi

start_another_proc_func_inner(){


	echo "$BASHPID" >> $tmp_pid_file
	$term_exec_string "$1"

}

start_another_proc_func(){

	sleep $sleep_time

	start_another_proc_func_inner "$1"&

}

pushd $directory


cat /dev/null >| $tmp_pid_file


start_another_proc_func "pushd $directory && pushd port_mapper && ./port_mapper.exe && exec bash"

start_another_proc_func "pushd $directory && pushd master_server && ./master_server.exe  && exec bash"

start_another_proc_func "pushd $directory && pushd server && ./server.exe  && exec bash"

start_another_proc_func "pushd $directory && pushd client &&  exec bash"

start_another_proc_func "pushd $directory && pushd server_browser &&  exec bash"

start_another_proc_func "pushd $directory && pushd master_server && bash ./no-ip-stuff.sh && exec bash"

start_another_proc_func "pushd $directory && exec bash"

cat $tmp_pid_file
