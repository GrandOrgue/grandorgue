#!/bin/bash

# $1 - Version
# $2 - Build version
# $3 - Go source Dir. If not set then relative to the script dir

set -e

DIR=$(readlink -f $(dirname $0))
source $DIR/../set-ver-prms.sh "$1" "$2"

if [[ -n "$3" ]]; then
	SRC_DIR=$(readlink -f $3)
else
	SRC_DIR=$(readlink -f $DIR/../..)
fi

PARALLEL_PRMS="-j$(nproc)"

mkdir -p build/appimage-x86_64
pushd build/appimage-x86_64

rm -rf *
export LANG=C

GO_PRMS="-DCMAKE_INSTALL_PREFIX=/usr \
  -DRTAUDIO_USE_JACK=OFF \
  -DRTMIDI_USE_JACK=OFF \
  -DGO_USE_JACK=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  $CMAKE_VERSION_PRMS"
echo "cmake -G \"Unix Makefiles\" $GO_PRMS . $SRC_DIR"
cmake -G "Unix Makefiles" $GO_PRMS . $SRC_DIR
make -k $PARALLEL_PRMS
make install DESTDIR=AppDir

# initialize AppDir and build AppImage
export DEPLOY_GTK_VERSION=3 APPIMAGE_EXTRACT_AND_RUN=1
linuxdeploy-x86_64.AppImage --appdir AppDir --plugin gtk
appimagetool-x86_64.AppImage --no-appstream AppDir grandorgue-$1-$2.x86_64.AppImage

popd
