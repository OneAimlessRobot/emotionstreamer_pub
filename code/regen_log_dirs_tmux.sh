#!/bin/bash


echo "Regenerating (tmux) log directories!!!"

echo "Deleting (tmux) log directories!!!"
bash ./rem_tmux_log_directories.sh

echo "Deleted (tmux) log directories!!!"

echo "Cre8ing (tmux) log directories!!!"
bash ./cre8_tmux_log_directories.sh
echo "Cre8ed (tmux) log directories!!!"
