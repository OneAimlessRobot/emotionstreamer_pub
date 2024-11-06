#!/bin/bash

if [ $# -ne 1 ]; 
then
	echo "Nao tens argumentos eiiaaadizer"
	exit -1
fi

rm "Includes/$1.h" "Sources/$1.c"
