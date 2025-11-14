#!/bin/bash

valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./client*exe play:alsa memories
