#!/bin/bash

#directory="/mnt/FASTstorage/FASTprogs/emotionstreamer/code"

directory=$(pwd)

#shell_init_string="xterm -e"
shell_init_string="foot -- bash -c"
$shell_init_string "pushd $directory && pushd port_mapper && ./port_mapper.exe && exec bash"&
$shell_init_string "pushd $directory && pushd server && ./server.exe  && exec bash"&
$shell_init_string "pushd $directory && pushd client &&  exec bash"&
$shell_init_string "pushd $directory && pushd master_server && bash ./no-ip-stuff.sh && exec bash"&
$shell_init_string "pushd $directory && exec bash"&
