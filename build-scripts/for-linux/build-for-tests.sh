#!/bin/bash

# $1 - target deb architecture. Default - current
# $2 - build type: Debug or Release. Default - Debug
# $3 - enable coverage: ON or OFF. Default - ON for Debug, OFF for Release
# $4 - optional: "asan" to enable AddressSanitizer

set -e

PARALLEL_PRMS="-j$(nproc)"

DIR=$(dirname $0)

SRC_DIR=$(readlink -f $(dirname $0)/../..)

TARGET_ARCH=${1:-$(dpkg --print-architecture)}
BUILD_TYPE=${2:-Debug}
COVERAGE=${3:-$([ "$BUILD_TYPE" = "Debug" ] && echo ON || echo OFF)}

export LANG=C

# Enable coverage only when explicitly requested (default: ON for Debug)
COVERAGE_FLAG=""
if [ "$COVERAGE" = "ON" ]; then
    COVERAGE_FLAG="-DGO_BUILD_COVERAGE=ON"
fi

ASAN_FLAG=""
if [ "${4:-}" = "asan" ]; then
    ASAN_FLAG="-DGO_BUILD_ASAN=ON"
fi

# Define BUILD_TESTING=ON
GO_PRMS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE -DBUILD_TESTING=ON $COVERAGE_FLAG $ASAN_FLAG \
  $CMAKE_VERSION_PRMS \
  $($DIR/cmake-prm-yaml-cpp.bash $TARGET_ARCH)"

BUILD_DIR="$SRC_DIR/build/tests"
mkdir -p "$BUILD_DIR"
pushd "$BUILD_DIR"

# Build simply GrandOrgue without packaging
echo "cmake -G \"Unix Makefiles\" $GO_PRMS . $SRC_DIR"
cmake -G "Unix Makefiles" $GO_PRMS . $SRC_DIR
echo "cmake --build . --parallel $PARALLEL_PRMS"
cmake --build . --parallel $PARALLEL_PRMS

popd
