#!/bin/bash

# $1 - directory
# $2 - target deb architecture. Default - current

set -e

PARALLEL_PRMS="-j$(nproc)"

# Install requirements for tests infrastructure
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y git gcovr

# Install cpptrace from sources
mkdir cpptrace
cd cpptrace
git clone https://github.com/jeremy-rifkin/cpptrace.git .
git checkout v0.4.0
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$PARALELL_PRMS
sudo make install
cd ../..

DIR=$(dirname $0)

SRC_DIR=$(readlink -f $(dirname $0)/../..)

TARGET_ARCH=${2:$(dpkg --print-architecture)}

export LANG=C

# Define BUILD_TESTING=ON
GO_PRMS="-DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
  $CMAKE_VERSION_PRMS \
  $($DIR/cmake-prm-yaml-cpp.bash $TARGET_ARCH)"

# Build simply GrandOrgue without packaging
echo "cmake -G \"Unix Makefiles\" $GO_PRMS . $SRC_DIR"
cmake -G "Unix Makefiles" $GO_PRMS . $SRC_DIR
echo "cmake --build . --parallel $PARALLEL_PRMS"
cmake --build . --parallel $PARALLEL_PRMS
