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

# do not add more directories to PKG_CONFIG_PATH because a bug in cmake that replaces ":" with ";"
# calculate MINGW_DIR for this linux distro
if [[ -d /usr/x86_64-w64-mingw32/sys-root/mingw ]]; then
  MINGW_DIR=/usr/x86_64-w64-mingw32/sys-root/mingw # redhat-based
else
  MINGW_DIR=/usr/x86_64-w64-mingw32 # debian-based
fi

PKG_CONFIG_PATH=$MINGW_DIR/lib/pkgconfig; export PKG_CONFIG_PATH;
WX_CONFIG=$MINGW_DIR/bin/wx-config; export WX_CONFIG

CC_EXE=/usr/bin/x86_64-w64-mingw32-gcc-posix
[ -f $CC_EXE ] || CC_EXE=/usr/bin/x86_64-w64-mingw32-gcc

CPP_EXE=/usr/bin/x86_64-w64-mingw32-g++-posix
[ -f $CPP_EXE ] || CPP_EXE=/usr/bin/x86_64-w64-mingw32-g++

LIBRARY_PATH="$MINGW_DIR/lib"
MINGW_PRMS="-DCMAKE_C_COMPILER=$CC_EXE \
  -DCMAKE_CXX_COMPILER=$CPP_EXE \
  -DCMAKE_RC_COMPILER=/usr/bin/x86_64-w64-mingw32-windres"

# additional dir for finding dlls. Only for debian-based
CPP_DLL_DIR=$(dirname $($CPP_EXE -v 2>&1 | grep COLLECT_LTO_WRAPPER= | sed 's/COLLECT_LTO_WRAPPER=//; s:-win32/:-posix/:'))
[[ ! -f $MINGW_DIR/bin/libstdc++-6.dll ]] && [[ -d "$CPP_DLL_DIR" ]] && MINGW_PRMS="$MINGW_PRMS -DDEPEND_ADD_DIRS=$CPP_DLL_DIR"

MINGW_PRMS="$MINGW_PRMS -DCMAKE_LIBRARY_PATH=$CPP_DLL_DIR;$LIBRARY_PATH"

GO_WIN_PRMS="-DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
  -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
  -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
  -DMSYS=1 -DSTATIC=1 \
  -DCMAKE_SYSTEM_PROCESSOR=x86_64 \
  -DCMAKE_VERBOSE_MAKEFILE=ON \
  -DRTAUDIO_USE_ASIO=ON -DASIO_SDK_DIR=/usr/local/asio-sdk \
  -DIMPORT_EXECUTABLES=../build-tools/ImportExecutables.cmake"
GO_PRMS="-DGO_USE_JACK=ON $CMAKE_VERSION_PRMS"

cmake -DINSTALL_DEPEND=ON $MINGW_PRMS $GO_WIN_PRMS $GO_PRMS . $SRC_DIR
make $PARALLEL_PRMS VERBOSE=1 package

#build debug symbols
mkdir -p pdb
export WINEPATH=Z:/usr/local/share/wine/msvc/VC/Tools/MSVC/14.29.30133/bin/Hostx86/x86
wine /usr/local/share/wine/cv2pdb/cv2pdb.exe bin/GrandOrgue.exe /tmp/GrandOrgue.exe pdb/GrandOrgue.pdb
wine /usr/local/share/wine/cv2pdb/cv2pdb.exe bin/GrandOrguePerfTest.exe /tmp/GrandOrguePerfTest.exe pdb/GrandOrguePerfTest.pdb
wine /usr/local/share/wine/cv2pdb/cv2pdb.exe bin/GrandOrgueTool.exe /tmp/GrandOrgueTool.exe pdb/GrandOrgueTool.pdb
wine /usr/local/share/wine/cv2pdb/cv2pdb.exe bin/libGrandOrgueCore.dll /tmp/libGrandOrgueCore.dll pdb/libGrandOrgueCore.pdb

pushd pdb
zip ../grandorgue-debug-$1-$2.windows.x86_64.zip *
popd

popd

