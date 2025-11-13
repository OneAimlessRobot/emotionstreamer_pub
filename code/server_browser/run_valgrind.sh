#!/bin/bash

valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./server_browser.exe showme localhost:11001
