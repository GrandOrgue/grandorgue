#!/bin/bash

# sets variables related to mingw cross compiling for the current linux distro
# MINGW_DIR - the basic mingw directory
# PKG_CONFIG_PATH - the configuration for pkg-config
# CC_COMPILER - the c compiler executable path
# CPP_COMPILER - the c++ compiler executable path
# CPP_DLL_DIR - the directory with the standard c++ libraries
# RC_COMPILER - the resource compiler executable path
# MINGW_LIBRARY_PATH - the directory with libraries
# CMAKE_MINGW_PRMS - the cmake parameters for cross-compiling

MINGW_DIR=/mingw64

# do not add more directories to PKG_CONFIG_PATH because a bug in cmake that replaces ":" with ";"
PKG_CONFIG_PATH="$MINGW_DIR/lib/pkgconfig"; export PKG_CONFIG_PATH;

CC_COMPILER=/usr/bin/x86_64-w64-mingw32-gcc-posix
[ -f $CC_COMPILER ] || CC_COMPILER=/usr/bin/x86_64-w64-mingw32-gcc

CPP_COMPILER=/usr/bin/x86_64-w64-mingw32-g++-posix
[ -f $CPP_COMPILER ] || CPP_COMPILER=/usr/bin/x86_64-w64-mingw32-g++

RC_COMPILER=/usr/bin/x86_64-w64-mingw32-windres

MINGW_LIBRARY_PATH="$MINGW_DIR/lib"
CMAKE_MINGW_PRMS="-DCMAKE_C_COMPILER=$CC_COMPILER \
  -DCMAKE_CXX_COMPILER=$CPP_COMPILER \
  -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
  -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
  -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
  -DCMAKE_LIBRARY_PATH=$MINGW_LIBRARY_PATH \
  -DCMAKE_PREFIX_PATH=$MINGW_DIR \
  -DCMAKE_RC_COMPILER=$RC_COMPILER \
  -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_SYSTEM_PROCESSOR=x86_64"
