#!/bin/bash

#directory="/mnt/FASTstorage/FASTprogs/emotionstreamer/code"

directory=$(pwd)

xterm -e "pushd $directory && pushd port_mapper && ./*exe && exec bash"&
xterm -e "pushd $directory && pushd server && ./*exe  && exec bash"&
xterm -e "pushd $directory && pushd client &&  exec bash"&
xterm -e "pushd $directory && pushd master_server && ./no-ip-stuff.sh && exec bash"&
xterm -e "pushd $directory && exec bash"&
