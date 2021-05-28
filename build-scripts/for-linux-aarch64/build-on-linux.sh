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

# Build imageconverter
mkdir -p build-for/build-tools
pushd build-for/build-tools
rm -rf *
cmake $SRC_DIR/src/build
make
popd

# Build for aarch64
mkdir -p build-for/linux-aarch64
pushd build-for/linux-aarch64
rm -rf *
export LANG=C
export PKG_CONFIG_PATH=/usr/lib/aarch64-linux-gnu/pkgconfig

GO_ARM_PRMS="-DCMAKE_SYSTEM_NAME=Linux \
  -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
  -DCMAKE_C_COMPILER=/usr/bin/aarch64-linux-gnu-gcc \
  -DCMAKE_CXX_COMPILER=/usr/bin/aarch64-linux-gnu-g++ \
  -DCPACK_DEBIAN_PACKAGE_ARCHITECTURE=aarch64 \
  -DCPACK_RPM_PACKAGE_ARCHITECTURE=aarch64 \
  -DIMPORT_EXECUTABLES=../build-tools/ImportExecutables.cmake"
GO_PRMS="-DCMAKE_BUILD_TYPE=Release $CMAKE_VERSION_PRMS"
echo "cmake -G \"Unix Makefiles\" $GO_PRMS . $SRC_DIR"
cmake -G "Unix Makefiles" $GO_PRMS $GO_ARM_PRMS . $SRC_DIR
make -k $PARALLEL_PRMS VERBOSE=1 package

rm -f *_all.deb *.noarch.rpm

# Replace libgcc-s1 dependency with libgcc1 
DEB_PACKAGE_NAME=$(find . -name "*_aarch64.deb" -print)
rm -rf deb-extracted 
mkdir deb-extracted
pushd deb-extracted
ar x "../$DEB_PACKAGE_NAME"
mkdir control-extracted
pushd control-extracted
tar xf "../control.tar.gz"
sed -Ei 's/libgcc-s1 \(>= [^)]+\)/libgcc1 (>= 1:8)/' control
tar cf ../control.tar.gz .
popd
rm -f "../$DEB_PACKAGE_NAME"
ar rcs "../$DEB_PACKAGE_NAME" debian-binary control.tar.gz data.tar.gz
popd

popd
