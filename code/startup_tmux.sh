#!/bin/bash


directory=$(pwd)

tmux new-sesstion -d -s my_stuff "pushd $directory && pushd port_mapper && ./*exe"
tmux new-window -t my_stuff:2 "pushd $directory && pushd server && ./*exe"
tmux new-window -t my_stuff:3 "pushd $directory && pushd client"
tmux attack -t my_stuff
