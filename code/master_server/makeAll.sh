#!/bin/bash

make cleanall && make server && make client -j 16 && make browser && make heartbeat
