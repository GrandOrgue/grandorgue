#!/bin/bash

# $1 - Version
# $2 - Build version
# $3 - Go source Dir. If not set then relative to the script dir
# $4 - pkg_suffix
# $5 - release flag (ON/OFF, default: OFF)

set -e

DIR=$(readlink -f $(dirname $0))

source $DIR/../set-ver-prms.sh "$1" "$2" "$5"

if [[ -n "$3" ]]; then
	SRC_DIR=$3
else
	SRC_DIR=$(readlink -f $DIR/../..)
fi

PARALLEL_PRMS="-j$(nproc)"

mkdir -p build/build-tools
pushd build/build-tools
rm -rf *
cmake $SRC_DIR/src/build
make
popd

mkdir -p build/win64
pushd build/win64

rm -rf *
export LANG=C

WX_CONFIG=$MINGW_DIR/bin/wx-config; export WX_CONFIG

source $DIR/set-mingw-vars.sh

CMAKE_WIN_PRMS="-DASIO_SDK_DIR=/usr/local/asio-sdk \
  -DCV2PDB_EXE=/usr/local/share/wine/cv2pdb/cv2pdb.exe \
  -DIMPORT_EXECUTABLES=../build-tools/ImportExecutables.cmake \
  -DINSTALL_DEPEND=ON \
  -DMSYS=1 -DSTATIC=1 \
  -DRTAUDIO_USE_ASIO=ON \
  -DVC_PATH=/usr/local/share/wine/msvc/VC/Tools/MSVC/14.29.30133/bin/Hostx86/x86"

CMAKE_APP_PRMS="-DGO_USE_JACK=ON $CMAKE_VERSION_PRMS $CMAKE_RELEASE_FLAG_PRM"

cmake $CMAKE_MINGW_PRMS $CMAKE_WIN_PRMS $CMAKE_APP_PRMS . $SRC_DIR
make $PARALLEL_PRMS VERBOSE=1 package

popd

