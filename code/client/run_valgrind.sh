#!/bin/bash

valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./client*exe play:oss re8abyss
#valgrind --track-fds=yes ./client*exe play:pulse ./mp3files/404

