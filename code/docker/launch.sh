#!/bin/bash

if [ $# -ne 3 ]
then
	echo "Usage: launch.sh <container name or id> <image name> <dir to mount>";
	exit 1
fi

sudo docker run --name $1 -ti -v "$3:/home" $2
