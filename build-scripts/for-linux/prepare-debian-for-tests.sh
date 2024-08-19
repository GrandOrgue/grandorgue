#!/bin/bash

set -e

DIR=`dirname $0`
$DIR/../for-linux/prepare-debian-based.sh wx32 amd64

# Install requirements for tests infrastructure
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y gcovr

# Install cpptrace from sources
if ! ls /usr/local/lib | grep libcpptrace &> /dev/null
then
    CPP_TRACE="0.4.1"
    CPP_TRACE_VERSION="v$CPP_TRACE"
    mkdir -p cpptrace
    pushd cpptrace
    wget https://github.com/jeremy-rifkin/cpptrace/archive/refs/tags/$CPP_TRACE_VERSION.tar.gz
    tar -xf $CPP_TRACE_VERSION.tar.gz
    cd cpptrace-$CPP_TRACE
    mkdir -p build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j$PARALELL_PRMS
    sudo make install
    popd
else
    echo "Cpptrace is already installed"
fi
