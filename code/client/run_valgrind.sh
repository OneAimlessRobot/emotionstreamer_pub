#!/bin/bash

valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./client*exe play:pulse intro_by_2xx
