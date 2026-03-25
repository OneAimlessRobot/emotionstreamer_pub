#!/bin/bash


echo "Regenerating log directories!!!"

echo "Deleting log directories!!!"
bash ./rem_log_directories.sh

echo "Deleted log directories!!!"

echo "Cre8ing log directories!!!"
bash ./cre8_log_directories.sh
echo "Cre8ed log directories!!!"
