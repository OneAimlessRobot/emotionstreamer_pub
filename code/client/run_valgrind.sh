#!/bin/bash

#valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./client*exe play:pulse ./mp3files/404
valgrind --track-fds=yes ./client*exe play:pulse ./mp3files/404

