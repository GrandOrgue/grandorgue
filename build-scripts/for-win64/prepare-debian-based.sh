#!/bin/bash

# $1 - wx version: empty or auto. wxWidgets 3.3.4 from WxWidgetsCross is used

set -e

BASE_DIR=$(dirname $0)
WX_VER=$1

case "${WX_VER:-auto}" in
auto) ;;
*)
  echo "Unsupported wx version $WX_VER: only auto is available" >&2
  exit 1
  ;;
esac

sudo dpkg --add-architecture i386
sudo apt-get update

# remove an odd version of packages that prevents installing wine32
$BASE_DIR/../for-linux/prepare-debian-based-align-libs.sh $TARGET_ARCH i386

sudo DEBIAN_FRONTEND=noninteractive apt-get install -y wget

$BASE_DIR/prepare-debian-based-mingw.sh

# install dependencies for wine32
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
  libgcc-s1:i386 libstdc++6:i386

sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
  wget unzip cmake g++ pkg-config nsis \
  docbook-xsl xsltproc gettext po4a imagemagick zip \
  wine32 winbind pipx

if ! command -v msys2dl &> /dev/null; then
  # Install a tool to download pre-compiled libraries from msys2 repositories
  sudo PIPX_HOME=/opt/pipx PIPX_BIN_DIR=/usr/local/bin \
    pipx install msys2dl==2.1.1
fi

if ! command -v cygpath &> /dev/null; then
  # Install the script that does nothing useful but is required by wx-config from MSYS2
  sudo mkdir -p /usr/local/bin
  sudo cp "$BASE_DIR"/cygpath /usr/local/bin/cygpath
fi

mkdir -p deb
pushd deb

# MSYS2 packages are built against newer libstdc++.
# It will be downloaded as a dependency for the packages below.
source $BASE_DIR/msys2-mingw64-packages.sh
PACKAGES_FROM_MSYS2=("${MSYS2_MINGW64_LIB_PACKAGES[@]}")
DEB_PACKAGES_FROM_MSYS2=("${PACKAGES_FROM_MSYS2[@]/%/-msys2-mingw64}")
if ! dpkg -l "${DEB_PACKAGES_FROM_MSYS2[@]}" &>/dev/null; then
  # Download packages from MSYS2 repositories and convert them to .deb.
  rm -rf ./msys2
  mkdir msys2
  # Exclude gcc-libs: it is an empty transitional metapackage pulled in
  # transitively (e.g. via readline->termcap), and msys2dl treats its
  # versioned CONFLICTS (e.g. "gcc-libs<16.2.0-4", declared by the
  # libgcc/libstdc++/libatomic/libgomp/libquadmath split) as unconditional,
  # so it reports a conflict even though the installed gcc-libs is the
  # newer, compatible version. Excluding it is safe since it carries no
  # files of its own.
  msys2dl make-deb --output ./msys2 --env mingw64 --exclude mingw-w64-x86_64-gcc-libs -- "${PACKAGES_FROM_MSYS2[@]}"
  sudo dpkg -i msys2/*.deb
fi

popd

# wxWidgets is no longer available in MSYS2 MINGW64, so use our own build
# from https://github.com/GrandOrgue/WxWidgetsCross

# remove the old wxWidgets from MSYS2 if it was installed earlier: it conflicts
# with the new one
OLD_WX_DEBS=$(dpkg-query -W -f='${Package}\n' 'wxwidgets3.2-*-msys2-mingw64' 2>/dev/null || true)
if [[ -n "$OLD_WX_DEBS" ]]; then
  sudo dpkg -r $OLD_WX_DEBS
fi

WX_CROSS_VERSION=3.3.4-1.go
WX_CROSS_INSTALLED_VERSION=$(dpkg-query -W -f='${Version}' wxwidgets3-mingw-w64 2>/dev/null || true)

if [[ "$WX_CROSS_INSTALLED_VERSION" != "$WX_CROSS_VERSION" ]]; then
  rm -rf deb/wx
  wget -P deb/wx \
    https://github.com/GrandOrgue/WxWidgetsCross/releases/download/${WX_CROSS_VERSION}/wxwidgets3-mingw-w64_${WX_CROSS_VERSION}_all.deb
  sudo DEBIAN_FRONTEND=noninteractive apt-get install -y ./deb/wx/*.deb
fi

# download and install ASIO sdk
$BASE_DIR/download-asio-sdk.sh

# download VC for wine
if [ ! -d /usr/local/share/wine/msvc ]; then
	DL_DIR=`mktemp -d -t vc.XXX`
	wget -O $DL_DIR/VC2019.zip https://github.com/GrandOrgue/DockerMsvcCpp/releases/download/0.1/VC2019.zip
	sudo mkdir -p /usr/local/share/wine/msvc.tmp
	sudo rm -rf /usr/local/share/wine/msvc.tmp/*
	sudo unzip -d /usr/local/share/wine/msvc.tmp $DL_DIR/VC2019.zip
	rm -rf $DL_DIR
	sudo mv /usr/local/share/wine/msvc.tmp /usr/local/share/wine/msvc
fi

# download cv2pdb
$BASE_DIR/download-cv2pdb.sh /usr/local/share/wine/cv2pdb
