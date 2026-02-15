#!/data/data/com.termux/files/usr/bin/bash

directory="$(pwd)/code"

pushd $directory

tmux new-session -d -s my_stuff "bash -lc 'cd \"./port_mapper\" && ./port_mapper.exe; exec bash'"

tmux new-window -t my_stuff:2 "bash -lc 'cd \"./server\" && ./server.exe; exec bash'"

tmux new-window -t my_stuff:3 "bash -lc 'cd \"./client\"; exec bash'"

tmux new-window -t my_stuff:4 "bash -lc 'bash ./edit_configs.sh; exec bash'"

tmux new-window -t my_stuff:5 "bash -lc 'cd \"./converter_tool\" ; exec bash'"

tmux attach -t my_stuff

popd
