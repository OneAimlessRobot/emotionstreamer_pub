#!/bin/bash

git stash && git pull origin tls_support && bash makeAll.sh
