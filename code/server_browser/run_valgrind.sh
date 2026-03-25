#!/bin/bash

valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./emotionstreamer_server_browser.exe showme 192.168.0.100:11007
