#!/bin/bash

make cleanall && make server -j 12 && make client -j 16 && make browser -j 12 && make heartbeat -j 12 && make master -j 12
#make cleanall && make master

