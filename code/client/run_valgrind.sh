#!/bin/bash

valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./client*exe play: ./Kichi_postcreator/re8abyss
#valgrind --track-fds=yes ./client*exe play:pulse ./mp3files/404

