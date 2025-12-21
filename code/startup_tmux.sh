#!/data/data/com.termux/files/usr/bin/bash

directory="$(pwd)"

tmux new-session -d -s my_stuff "bash -lc 'cd \"$directory/port_mapper\" && ./port_mapper.exe; exec bash'"

tmux new-window -t my_stuff:2 "bash -lc 'cd \"$directory/server\" && ./server.exe; exec bash'"

tmux new-window -t my_stuff:3 "bash -lc 'cd \"$directory/client\"; exec bash'"

tmux attach -t my_stuff
