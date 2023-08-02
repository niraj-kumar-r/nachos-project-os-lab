#!/bin/bash
set -e

# sudo apt-get install gcc g++ gcc-multilib g++-multilib make
sudo dnf install gcc gcc-c++ glibc-devel.i686 glibc-devel.x86_64 libstdc++-devel.i686 libstdc++-devel.x86_64 make


cd ..
wget https://www.fit.hcmus.edu.vn/~ntquan/os/assignment/mips-decstation.linux-xgcc.gz
tar zxvf mips-decstation.linux-xgcc.gz
