#!/bin/bash

# $1 - target deb architecture. Default - current
# $2 - build type: Debug or Release. Default - Debug

set -e

PARALLEL_PRMS="-j$(nproc)"

DIR=$(dirname $0)

SRC_DIR=$(readlink -f $(dirname $0)/../..)

TARGET_ARCH=${1:-$(dpkg --print-architecture)}
BUILD_TYPE=${2:-Debug}

export LANG=C

# Enable coverage only for Debug builds
COVERAGE_FLAG=""
if [ "$BUILD_TYPE" = "Debug" ]; then
    COVERAGE_FLAG="-DGO_BUILD_COVERAGE=ON"
fi

# Define BUILD_TESTING=ON
GO_PRMS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE -DBUILD_TESTING=ON $COVERAGE_FLAG \
  $CMAKE_VERSION_PRMS \
  $($DIR/cmake-prm-yaml-cpp.bash $TARGET_ARCH)"

# Build simply GrandOrgue without packaging
echo "cmake -G \"Unix Makefiles\" $GO_PRMS . $SRC_DIR"
cmake -G "Unix Makefiles" $GO_PRMS . $SRC_DIR
echo "cmake --build . --parallel $PARALLEL_PRMS"
cmake --build . --parallel $PARALLEL_PRMS
