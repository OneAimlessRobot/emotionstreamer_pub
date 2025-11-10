#!/bin/bash

wget –content-disposition "https://www.noip.com/download/linux/latest"
tar xf noip-duc_3.1.1.tar.gz 
sudo dpkg -i noip-duc_3.1.1/binaries/noip-duc_3.1.1_amd64.deb 
sudo apt install -yf
