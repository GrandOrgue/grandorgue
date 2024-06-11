#!/bin/bash

set -e

# Install requirements for tests infrastructure
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y gcovr

# Install cpptrace from sources
CPP_TRACE="0.4.1"
CPP_TRACE_VERSION="v$CPP_TRACE"
mkdir cpptrace
cd cpptrace
wget https://github.com/jeremy-rifkin/cpptrace/archive/refs/tags/$CPP_TRACE_VERSION.tar.gz
tar -xf $CPP_TRACE_VERSION.tar.gz
cd cpptrace-$CPP_TRACE
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$PARALELL_PRMS
sudo make install
cd ../..
