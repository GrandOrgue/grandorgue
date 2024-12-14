#!/bin/bash

set -e

CPP_TRACE_VERSION="0.4.1"

# Install cpptrace from sources
if ! ls /usr/local/lib*/libcpptrace.a &> /dev/null ; then
    BUILD_DIR=`mktemp -d`
    pushd $BUILD_DIR
    wget https://github.com/jeremy-rifkin/cpptrace/archive/refs/tags/v$CPP_TRACE_VERSION.tar.gz
    tar -xf v$CPP_TRACE_VERSION.tar.gz
    cd cpptrace-$CPP_TRACE_VERSION
    mkdir -p build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j`nproc`
    sudo make install
    popd
    rm -rf $BUILD_DIR
else
    echo "Cpptrace is already installed"
fi
