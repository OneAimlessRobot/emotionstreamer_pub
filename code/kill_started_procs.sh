#!/bin/bash

sleep_time=0.5

directory=$(pwd)

session_name="my_stuff"

tmux kill-session -t "${session_name}"

