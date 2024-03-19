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

# Find VC (needed for CV2PDB)
VCTOOLS_COMPONENT="Microsoft.VisualStudio.Component.VC.Tools.x86.x64"
VCTOOLS_DIR=$(vswhere -latest -requires "$VCTOOLS_COMPONENT" -find '**\bin\Hostx86\x64')
if [ -z "${VCTOOLS_DIR}" ]; then
	echo "Could not find $VCTOOLS_COMPONENT Visual Studio component"
	exit 1
fi

# Prepare build directory
mkdir -p build/win64
pushd build/win64
rm -rf *
export LANG=C

# Run cmake & make
CMAKE_WIN_PRMS=(-DINSTALL_DEPEND=ON -DSTATIC=1 -DASIO_SDK_DIR=/usr/local/asio-sdk -DRTAUDIO_USE_ASIO=ON -G "MSYS Makefiles"
				-DCV2PDB_EXE=/usr/local/share/cv2pdb/cv2pdb.exe "-DVC_PATH=$VCTOOLS_DIR")
CMAKE_APP_PRMS=(-DGO_USE_JACK=ON $CMAKE_VERSION_PRMS)
CMAKE_PRMS=("${CMAKE_WIN_PRMS[@]}" "${CMAKE_APP_PRMS[@]}" "$SRC_DIR")

echo "> Running CMake with parameters:" "${CMAKE_PRMS[@]}"
cmake "${CMAKE_PRMS[@]}"
make $PARALLEL_PRMS VERBOSE=1 package

popd
