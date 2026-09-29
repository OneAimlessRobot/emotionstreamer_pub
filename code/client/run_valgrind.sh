#!/bin/bash

#valgrind --leak-check=full --show-leak-kinds=all ./*client*exe play:alsa "./dawid"
valgrind  --show-leak-kinds=all ./*client*exe play:alsa "./dawid"
#valgrind  ./*client*exe play: "./dawid"
#valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./*client*exe peek Ugly*Witchery*
#valgrind --track-fds=yes ./client*exe play:pulse ./mp3files/404

