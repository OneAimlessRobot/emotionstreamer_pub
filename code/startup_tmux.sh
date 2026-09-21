#!/data/data/com.termux/files/usr/bin/bash

sleep_time=0.5

sleep_time_mult_attach_cmd_proc=0.5

curr_term_started=1

session_name="my_stuff"

directory="$(pwd)"



start_another_proc_func_inner(){

	tmux new-window -t "${session_name}":"${curr_term_started}" "bash -lc \"${1}\""
	curr_term_started=$(("${curr_term_started}"+1))

}

start_another_proc_func(){

	sleep "${sleep_time}"

	start_another_proc_func_inner "$1; exec bash"

}

pushd "${directory}"

tmux new-session -d -s "${session_name}"

start_another_proc_func "pushd ${directory}/port_mapper ; ./emotionstreamer_port_mapper.exe >./tmux_logs/log_file_port_mapper_$(date +%d-%m-%Y_%H:%M:%S).txt 2>./tmux_logs/error_log_file_port_mapper_$(date +%d-%m-%Y_%H:%M:%S).txt"

#start_another_proc_func "pushd ${directory}/master_server ; ./emotionstreamer_master_server.exe >./tmux_logs/log_file_master_server_$(date +%d-%m-%Y_%H:%M:%S).txt 2>./tmux_logs/error_log_file_master_server_$(date +%d-%m-%Y_%H:%M:%S).txt"

#start_another_proc_func "pushd ${directory}/heartbeat_server ; ./emotionstreamer_heartbeat_server.exe >./tmux_logs/log_file_heartbeat_server_$(date +%d-%m-%Y_%H:%M:%S).txt 2>./tmux_logs/error_log_file_heartbeat_server_$(date +%d-%m-%Y_%H:%M:%S).txt"

#start_another_proc_func "pushd ${directory}/content_server ; bash start_content_server.sh >./tmux_logs/log_file_content_server_$(date +%d-%m-%Y_%H:%M:%S).txt 2>./tmux_logs/error_log_file_content_server_$(date +%d-%m-%Y_%H:%M:%S).txt"

start_another_proc_func "pushd ${directory}/client"

start_another_proc_func "pushd ${directory}/server_browser"

#start_another_proc_func "bash ./edit_configs.sh"

#start_another_proc_func "pushd ${directory}/converter_tool"

sleep $(echo "${sleep_time}*${sleep_time_mult_attach_cmd_proc}" | bc) ; tmux attach -t "${session_name}"

popd
