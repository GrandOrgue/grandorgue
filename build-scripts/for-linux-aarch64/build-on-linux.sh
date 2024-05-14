#!/bin/bash

# $1 - Version
# $2 - Build version
# $3 - Go source Dir. If not set then relative to the script dir
# $4 - package suffix: empty or wx30

set -e

DIR=$(readlink -f $(dirname $0))
source $DIR/../set-ver-prms.sh "$1" "$2"

if [[ -n "$3" ]]; then
	SRC_DIR=$3
else
	SRC_DIR=$(readlink -f $(dirname $0)/../..)
fi
PACKAGE_SUFFIX=$4

PARALLEL_PRMS="-j$(nproc)"

# Build imageconverter
mkdir -p build/build-tools
pushd build/build-tools
rm -rf *
cmake $SRC_DIR/src/build
make
popd

# Build for aarch64
mkdir -p build/linux-aarch64
pushd build/linux-aarch64
rm -rf *
export LANG=C
export PKG_CONFIG_PATH=/usr/lib/aarch64-linux-gnu/pkgconfig

GO_ARM_PRMS="-DCMAKE_SYSTEM_NAME=Linux \
  -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
  -DCMAKE_C_COMPILER=/usr/bin/aarch64-linux-gnu-gcc \
  -DCMAKE_CXX_COMPILER=/usr/bin/aarch64-linux-gnu-g++ \
  -DCPACK_DEBIAN_PACKAGE_ARCHITECTURE=arm64 \
  -DCPACK_RPM_PACKAGE_ARCHITECTURE=aarch64 \
  -DIMPORT_EXECUTABLES=../build-tools/ImportExecutables.cmake"

GO_PRMS="-DCMAKE_BUILD_TYPE=Release \
  $CMAKE_VERSION_PRMS \
  -DCMAKE_PACKAGE_SUFFIX=$PACKAGE_SUFFIX \
  $($DIR/../for-linux/cmake-prm-yaml-cpp.bash arm64)"

echo "cmake -G \"Unix Makefiles\" $GO_PRMS . $SRC_DIR"
cmake -G "Unix Makefiles" $GO_PRMS $GO_ARM_PRMS . $SRC_DIR
make -k $PARALLEL_PRMS VERBOSE=1 package

rm -f *_all.deb *.noarch.rpm

popd
