#!/bin/bash

valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./server_browser.exe showme 192.168.1.5:11000
