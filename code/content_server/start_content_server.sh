#!/bin/bash

executable_name="emotionstreamer_content_server.exe"
set server_tmp_dir_path
server_tmp_dir_path="/tmp/emotionstreamer_tmp_dir_path_thing_mabobber"
#sudo setcap 'cap_net_bind_service=+ep' $(pwd)/"${executable_name}"

rm -rf "${server_tmp_dir_path}"
server_tmp_dir_path="${server_tmp_dir_path}" $(pwd)/"${executable_name}"
rm -rf "${server_tmp_dir_path}"


