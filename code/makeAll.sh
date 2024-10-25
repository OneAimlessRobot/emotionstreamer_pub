#!/bin/bash

make cleanall && make server -j 16 && make client -j 16
