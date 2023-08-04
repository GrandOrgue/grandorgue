#!/bin/bash

# $1 - directory
# $2 - target deb architecture. Default - current

set -e

DIR=$(dirname $0)
# source $DIR/../set-ver-prms.sh "$1" "$2"

SRC_DIR=$(readlink -f $(dirname $0)/../..)


# PACKAGE_SUFFIX=$4
TARGET_ARCH=${2:$(dpkg --print-architecture)}

PARALLEL_PRMS="-j$(nproc)"

mkdir -p build/linux
pushd build/linux

rm -rf *
export LANG=C

GO_PRMS="-DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
  $CMAKE_VERSION_PRMS \
  $($DIR/cmake-prm-yaml-cpp.bash $TARGET_ARCH)"

echo "cmake -G \"Unix Makefiles\" $GO_PRMS . $SRC_DIR"
cmake -G "Unix Makefiles" $GO_PRMS . $SRC_DIR
make -k $PARALLEL_PRMS VERBOSE=1 package

popd
