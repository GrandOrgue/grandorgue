#!/bin/bash

set -e

BASE_DIR=$(dirname $0)

sudo dpkg --add-architecture i386
sudo apt-get update

# remove an odd version of packages that prevents installing wine32
$BASE_DIR/../for-linux/prepare-debian-based-align-libs.sh $TARGET_ARCH i386

# install dependencies for wine32
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
  libgcc-s1:i386 libstdc++6:i386

sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
  wget unzip cmake g++ pkg-config g++-mingw-w64-x86-64 nsis \
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
PACKAGES_FROM_MSYS2=(curl-winssl fftw jack2 wavpack yaml-cpp wxwidgets3.2-msw)
DEB_PACKAGES_FROM_MSYS2=("${PACKAGES_FROM_MSYS2[@]/%/-msys2-mingw64}")
if ! dpkg -l "${DEB_PACKAGES_FROM_MSYS2[@]}" &>/dev/null; then
  # Download packages from MSYS2 repositories and convert them to .deb.
  rm -rf ./msys2
  mkdir msys2
  msys2dl make-deb --output ./msys2 --env mingw64 -- "${PACKAGES_FROM_MSYS2[@]}"
  sudo dpkg -i msys2/*.deb
fi

popd

# download and install ASIO sdk
if [ ! -d /usr/local/asio-sdk ]; then
	DL_DIR=`mktemp -d -t asio.XXX`
	wget -O $DL_DIR/asiosdk.zip https://www.steinberg.net/asiosdk
	sudo unzip -o $DL_DIR/asiosdk.zip -d /usr/local/
	rm -rf $DL_DIR
	SDK_DIR=`ls -1d /usr/local/asiosdk* | tail -1`
	sudo ln -sf `basename $SDK_DIR` /usr/local/asio-sdk
fi

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
if [ ! -d /usr/local/share/wine/cv2pdb ]; then
	DL_DIR=`mktemp -d -t cv2pdb.XXX`
	wget -O $DL_DIR/cv2pdb-0.51.zip https://github.com/rainers/cv2pdb/releases/download/v0.51/cv2pdb-0.51.zip
	sudo mkdir -p /usr/local/share/wine/cv2pdb.tmp
	sudo rm -rf /usr/local/share/wine/cv2pdb.tmp/*
	sudo unzip -d /usr/local/share/wine/cv2pdb.tmp $DL_DIR/cv2pdb-0.51.zip
	rm -rf $DL_DIR
	sudo mv /usr/local/share/wine/cv2pdb.tmp /usr/local/share/wine/cv2pdb
fi
