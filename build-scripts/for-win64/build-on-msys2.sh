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

# GrandOrgue itself is always compiled with the mingw-w64 gcc from MSYS2, not
# MSVC. cv2pdb is the only reason MSVC tools are used at all: it converts the
# mingw/DWARF debug info into a native .pdb file (a proprietary Microsoft
# format), which needs MSVC's own mspdb*.dll to write. This is purely for
# the optional GO_SPLIT_DEBUG_SYMBOLS feature, so it's best-effort - a
# missing Visual Studio install just skips .pdb splitting, not the build.
VCTOOLS_COMPONENT="Microsoft.VisualStudio.Component.VC.Tools.x86.x64"
VS_INSTALL_PATH=$(vswhere.exe -latest -requires "$VCTOOLS_COMPONENT" -property installationPath)
CMAKE_CV2PDB_PRMS=
if [ -n "$VS_INSTALL_PATH" ]; then
	VCTOOLS_DIR=$(ls -1d "$VS_INSTALL_PATH"/VC/Tools/MSVC/*/bin/Hostx86/x64 2>/dev/null | sort | tail -n 1)
fi
if [ -n "${VCTOOLS_DIR:-}" ]; then
	CMAKE_CV2PDB_PRMS="-DCV2PDB_EXE=/usr/local/share/cv2pdb/cv2pdb.exe -DVC_PATH=$VCTOOLS_DIR"
else
	echo "Warning: could not find the $VCTOOLS_COMPONENT Visual Studio component." >&2
	echo "Building without split debug symbols (.pdb). Install Visual Studio" >&2
	echo "Build Tools with the \"Desktop development with C++\" workload to enable it." >&2
fi

mkdir -p build/win64
pushd build/win64

rm -rf *
export LANG=C

CMAKE_WIN_PRMS="-DASIO_SDK_DIR=/usr/local/asio-sdk \
  -DINSTALL_DEPEND=ON \
  -DSTATIC=1 \
  -DRTAUDIO_USE_ASIO=ON \
  $CMAKE_CV2PDB_PRMS"

CMAKE_APP_PRMS="-DGO_USE_JACK=ON $CMAKE_VERSION_PRMS $CMAKE_RELEASE_FLAG_PRM"

cmake -G "MSYS Makefiles" $CMAKE_WIN_PRMS $CMAKE_APP_PRMS . "$SRC_DIR"
make $PARALLEL_PRMS VERBOSE=1 package

popd
