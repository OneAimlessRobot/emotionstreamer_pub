#!/bin/bash

valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./server_browser.exe showme_master 192.168.1.2:11006
