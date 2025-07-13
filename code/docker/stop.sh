#!/bin/bash

if [ $# -ne 1 ]
then
	echo "Usage: $0 <container name or id>";
	exit 1
fi

sudo docker stop $1

echo "Container stopped!!!!!!"


sudo docker rm -f $1

echo "Container removed!!!!!!"
