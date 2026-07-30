#!/bin/bash

# Builds GrandOrgue for Windows natively, inside an MSYS2 MINGW64 shell.
# Meant to be run via build-on-windows.cmd.

# $1 - Version
# $2 - Build version
# $3 - Go source Dir. If not set then relative to the script dir
# $4 - pkg_suffix (unused for now, accepted for parity with build-on-linux.sh)
# $5 - release flag (ON/OFF, default: OFF)

set -e

DIR=$(dirname "$(readlink -f "$0")")

source "$DIR/../set-ver-prms.sh" "$1" "$2" "$5"

if [[ -n "$3" ]]; then
	SRC_DIR=$3
else
	SRC_DIR=$(readlink -f "$DIR/../..")
fi

PARALLEL_PRMS="-j$(nproc)"

# Find the VC tools needed by cv2pdb
VCTOOLS_COMPONENT="Microsoft.VisualStudio.Component.VC.Tools.x86.x64"
VCTOOLS_DIR=$(vswhere.exe -latest -requires "$VCTOOLS_COMPONENT" -find '**\bin\Hostx86\x64' | head -n 1)
if [ -z "$VCTOOLS_DIR" ]; then
	echo "Could not find $VCTOOLS_COMPONENT Visual Studio component" >&2
	echo "Install Visual Studio Build Tools with the \"Desktop development with C++\" workload" >&2
	exit 1
fi

mkdir -p build/win64
pushd build/win64

rm -rf *
export LANG=C

CMAKE_WIN_PRMS="-DASIO_SDK_DIR=/usr/local/asio-sdk \
  -DCV2PDB_EXE=/usr/local/share/cv2pdb/cv2pdb.exe \
  -DINSTALL_DEPEND=ON \
  -DSTATIC=1 \
  -DRTAUDIO_USE_ASIO=ON \
  -DVC_PATH=$VCTOOLS_DIR"

CMAKE_APP_PRMS="-DGO_USE_JACK=ON $CMAKE_VERSION_PRMS $CMAKE_RELEASE_FLAG_PRM"

cmake -G "MSYS Makefiles" $CMAKE_WIN_PRMS $CMAKE_APP_PRMS . "$SRC_DIR"
make $PARALLEL_PRMS VERBOSE=1 package

popd
