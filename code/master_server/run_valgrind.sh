#!/bin/bash

valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./*master_server.exe
