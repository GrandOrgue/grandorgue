#!/bin/bash

set -e
sudo apt update
sudo DEBIAN_FRONTEND=noninteractive apt install -y \
  wget unzip cmake g++ pkg-config g++-mingw-w64-x86-64 nsis \
  docbook-xsl xsltproc gettext po4a imagemagick \
  libz-mingw-w64-dev

mkdir -p deb
pushd deb

wget \
  https://github.com/GrandOrgue/Mingw64LibGnuRx/releases/download/2.6.1-1.os/libgnurx-mingw-w64_2.6.1-1.os_all.deb \
  https://launchpad.net/~tobydox/+archive/ubuntu/mingw-w64/+files/fftw-mingw-w64_3.3.6-3_all.deb \
  https://github.com/GrandOrgue/JackCross/releases/download/1.9.19-1.os/jack-mingw-w64_1.9.19-1.os_all.deb \
  https://github.com/GrandOrgue/WavPackCross/releases/download/5.4.0-1.go/wavpack-mingw-w64_5.4.0-1.go_all.deb \
  https://github.com/GrandOrgue/WxWidgetsCross/releases/download/3.0.2-2.go/wxwidgets3.0-mingw-w64_3.0.2-2.go_all.deb

sudo apt-get install -y \
  ./libgnurx-mingw-w64_2.6.1-1.os_all.deb \
  ./fftw-mingw-w64_3.3.6-3_all.deb \
  ./jack-mingw-w64_1.9.19-1.os_all.deb \
  ./wavpack-mingw-w64_5.4.0-1.go_all.deb \
  ./wxwidgets3.0-mingw-w64_3.0.2-2.go_all.deb

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
