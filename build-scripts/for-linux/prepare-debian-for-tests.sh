#!/bin/bash

# Install requirements for tests infrastructure
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y git gcovr

# Install cpptrace from sources
mkdir cpptrace
cd cpptrace
git clone https://github.com/jeremy-rifkin/cpptrace.git .
git checkout v0.4.0
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$PARALELL_PRMS
sudo make install
cd ../..
