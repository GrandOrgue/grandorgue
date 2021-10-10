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

mkdir -p build-for/appimage
pushd build-for/appimage

rm -rf *
export LANG=C

GO_PRMS="-DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release $CMAKE_VERSION_PRMS"
echo "cmake -G \"Unix Makefiles\" $GO_PRMS . $SRC_DIR"
cmake -G "Unix Makefiles" $GO_PRMS . $SRC_DIR
make -k $PARALLEL_PRMS
make install DESTDIR=AppDir

# Build AppImage using linuxdeploy and appimagetool that must be downloaded
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage

# make them executable
chmod +x linuxdeploy*.AppImage
chmod +x appimagetool*.AppImage

# initialize AppDir and build AppImage
./linuxdeploy-x86_64.AppImage --appdir AppDir
./appimagetool-x86_64.AppImage --no-appstream AppDir grandorgue-$1-$2-x86_64.AppImage

popd
