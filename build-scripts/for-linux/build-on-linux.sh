#!/bin/bash

# $1 - Version
# $2 - Build version
# $3 - Go source Dir. If not set then relative to the script dir
# $4 - package suffix: empty or wx30 or wx32
# $5 - target deb architecture. Default - current

set -e

DIR=$(readlink -f $(dirname $0))
source $DIR/../set-ver-prms.sh "$1" "$2"

if [[ -n "$3" ]]; then
	SRC_DIR=$3
else
	SRC_DIR=$(readlink -f $(dirname $0)/../..)
fi

PACKAGE_SUFFIX=$4
TARGET_ARCH=${5:$(dpkg --print-architecture)}

PARALLEL_PRMS="-j$(nproc)"

mkdir -p build/linux
pushd build/linux

rm -rf *
export LANG=C

GO_PRMS="-DCMAKE_BUILD_TYPE=Release \
  $CMAKE_VERSION_PRMS \
  -DCMAKE_PACKAGE_SUFFIX=$PACKAGE_SUFFIX \
  $($DIR/cmake-prm-yaml-cpp.bash $TARGET_ARCH)"

# a workaround of the new dpkg-shlibdeps that prevents cpack from making the DEB package
if ! dpkg-shlibdeps --help 1>/dev/null; then
  echo "set(IGNORE_MISSING_INFO_FLAG --ignore-missing-info)" >CPackSpkgShlibdeps.cmake
  GO_PRMS="$GO_PRMS -DCPACK_PROPERTIES_FILE=$(readlink -f CPackSpkgShlibdeps.cmake)"
fi

echo "cmake -G \"Unix Makefiles\" $GO_PRMS . $SRC_DIR"
cmake -G "Unix Makefiles" $GO_PRMS . $SRC_DIR
make -k $PARALLEL_PRMS VERBOSE=1 package

# generate source rpm
cpack -G RPM --config ./CPackSourceConfig.cmake

popd
