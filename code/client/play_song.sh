#!/bin/bash

#get song with pattern:

pattern="Elton"

result=$(./client.exe peek "${pattern}" | grep "${pattern}")

echo "${result}"


backend="oss"

./client.exe play:${backend} ${result}
