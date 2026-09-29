#!/bin/bash

pushd .. ; nano Make* ; make cleanclient -j 3; make client -j 3 ; popd
