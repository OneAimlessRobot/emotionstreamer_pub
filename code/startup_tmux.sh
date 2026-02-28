#!/data/data/com.termux/files/usr/bin/bash

sleep_time=0.5

sleep_time_mult_attach_cmd_proc=0.5

curr_term_started=1

session_name="my_stuff"

termux_tmp_pid_file=".termux_tmp_pids"

directory="$(pwd)"



start_another_proc_func_inner(){

	tmux new-window -t "${session_name}":"${curr_term_started}" "$1"
	echo $(tmux list-panes -t "${session_name}":"${curr_term_started}" -F '#{pane_pid}') >> "${termux_tmp_pid_file}"
	curr_term_started=$(("${curr_term_started}"+1))

}

start_another_proc_func(){

	sleep "${sleep_time}"

	start_another_proc_func_inner "$1"

}

if [ -f "${termux_tmp_pid_file}"  ]
then
	chmod a+rwx "${termux_tmp_pid_file}"
fi
pushd "${directory}"

tmux new-session -d -s "${session_name}"

start_another_proc_func "bash -lc 'cd \"./port_mapper\" && ./emotionstreamer_port_mapper.exe >./tmux_logs/log_file_port_mapper_$(date +%d-%m-%Y_%H:%M:%S).txt 2>&1 ; exec bash'"

start_another_proc_func "bash -lc 'cd \"./master_server\" && ./emotionstreamer_master_server.exe >./tmux_logs/log_file_master_server_$(date +%d-%m-%Y_%H:%M:%S).txt 2>&1 ; exec bash'"

start_another_proc_func "bash -lc 'cd \"./heartbeat_server\" && ./emotionstreamer_heartbeat_server.exe >./tmux_logs/log_file_heartbeat_server_$(date +%d-%m-%Y_%H:%M:%S).txt 2>&1 ; exec bash'"

start_another_proc_func "bash -lc 'cd \"./content_server\" && ./emotionstreamer_content_server.exe >./tmux_logs/log_file_content_server_$(date +%d-%m-%Y_%H:%M:%S).txt 2>&1 ; exec bash'"

start_another_proc_func "bash -lc 'cd \"./client\"; exec bash'"

start_another_proc_func "bash -lc 'cd \"./server_browser\"; exec bash'"

start_another_proc_func "bash -lc 'bash ./edit_configs.sh; exec bash'"

start_another_proc_func "bash -lc 'cd \"./converter_tool\" ; exec bash'"

sleep $(echo "${sleep_time}*${sleep_time_mult_attach_cmd_proc}" | bc) && tmux attach -t "${session_name}"

cat "${termux_tmp_pid_file}"

popd
