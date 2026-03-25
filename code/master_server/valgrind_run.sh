#!/bin/bash

valgrind --max-stackframe=6256368 --show-leak-kinds=all --track-fds=yes --leak-check=full ./*.exe

