#!/bin/bash

# $1 - Version
# $2 - Build version
# $3 - Go source Dir. If not set then relative to the script dir

set -e

source $(dirname $0)/../set-ver-prms.sh "$1" "$2"

if [[ -n "$3" ]]; then
	SRC_DIR=$3
else
	SRC_DIR=$(readlink -f $(dirname $0)/../..)
fi

PARALLEL_PRMS="-j$(nproc)"

mkdir -p build-tools
rm -rf build-tools/*
pushd build-tools
cmake $SRC_DIR/src/build
make
popd

mkdir -p build-win64
pushd build-win64

rm -rf *
export LANG=C

# do not add more directories to PKG_CONFIG_PATH because a bug in cmake that replaces ":" with ";"
PKG_CONFIG_PATH=/usr/x86_64-w64-mingw32/sys-root/mingw/lib/pkgconfig; export PKG_CONFIG_PATH; 
WX_CONFIG=/usr/x86_64-w64-mingw32/sys-root/mingw/bin/wx-config; export WX_CONFIG

CPP_EXE=/usr/bin/x86_64-w64-mingw32-g++-posix
[ -f $CPP_EXE ] || CPP_EXE=/usr/bin/x86_64-w64-mingw32-g++

MINGW_PRMS="-DCMAKE_C_COMPILER=/usr/bin/x86_64-w64-mingw32-gcc \
  -DCMAKE_CXX_COMPILER=$CPP_EXE \
  -DCMAKE_RC_COMPILER=/usr/bin/x86_64-w64-mingw32-windres"
  
GO_WIN_PRMS="-DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
  -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
  -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
  -DMSYS=1 -DSTATIC=1 \
  -DCMAKE_VERBOSE_MAKEFILE=ON \
  -DIMPORT_EXECUTABLES=../build-tools/ImportExecutables.cmake"
GO_PRMS="-DRTAUDIO_USE_ASIO=OFF -DGO_USE_JACK=OFF $CMAKE_VERSION_PRMS"

cmake -DINSTALL_DEPEND=ON $MINGW_PRMS $GO_WIN_PRMS $GO_PRMS . $SRC_DIR
make $PARALLEL_PRMS VERBOSE=1 package

popd

