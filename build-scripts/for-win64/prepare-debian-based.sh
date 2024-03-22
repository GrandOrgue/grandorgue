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
  docbook-xsl xsltproc gettext po4a imagemagick zip libz-mingw-w64-dev \
  wine32 winbind pipx git

if ! command -v msys2-download &> /dev/null; then
  # Install a tool to download pre-compiled libraries from msys2 repositories
  sudo PIPX_HOME=/opt/pipx PIPX_BIN_DIR=/usr/local/bin \
    pipx install git+https://github.com/nanoufo/msys2-downloader.git@v1.0.0
fi

mkdir -p deb
pushd deb

if ! dpkg -l curl-winssl-msys2-mingw64 &>/dev/null; then
  # Download curl and dependencies from MSYS2 repositories.
  # GO uses libcurl for checking for updates.
  rm -rf ./msys2
  mkdir msys2
  msys2-download --make-deb --output ./msys2 --env mingw64 --exclude gcc-libs zlib -- curl-winssl
  sudo dpkg -i msys2/*.deb
fi

wget \
  https://github.com/GrandOrgue/Mingw64LibGnuRx/releases/download/2.6.1-1.os/libgnurx-mingw-w64_2.6.1-1.os_all.deb \
  https://launchpad.net/~tobydox/+archive/ubuntu/mingw-w64/+files/fftw-mingw-w64_3.3.6-3_all.deb \
  https://github.com/GrandOrgue/JackCross/releases/download/1.9.19-1.os/jack-mingw-w64_1.9.19-1.os_all.deb \
  https://github.com/GrandOrgue/WavPackCross/releases/download/5.4.0-1.go/wavpack-mingw-w64_5.4.0-1.go_all.deb \
  https://github.com/GrandOrgue/WxWidgetsCross/releases/download/3.2.3-1.go/wxwidgets3-mingw-w64_3.2.3-1.go_all.deb \
  https://github.com/GrandOrgue/YamlCppAdd/releases/download/0.6.2-6.go/libyaml-cpp-mingw-w64_0.6.2-6.go_all.deb

sudo apt-get install -y \
  ./libgnurx-mingw-w64_2.6.1-1.os_all.deb \
  ./fftw-mingw-w64_3.3.6-3_all.deb \
  ./jack-mingw-w64_1.9.19-1.os_all.deb \
  ./wavpack-mingw-w64_5.4.0-1.go_all.deb \
  ./wxwidgets3-mingw-w64_3.2.3-1.go_all.deb \
  ./libyaml-cpp-mingw-w64_0.6.2-6.go_all.deb

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
