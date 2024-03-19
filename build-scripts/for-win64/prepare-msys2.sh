#!/bin/bash

set -eu

# Install build dependencies
pacman -S --needed --noconfirm \
	base-devel wget unzip \
	mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc mingw-w64-x86_64-fftw mingw-w64-x86_64-imagemagick \
	mingw-w64-x86_64-inkscape mingw-w64-x86_64-jack2 mingw-w64-x86_64-librsvg mingw-w64-x86_64-nsis \
	mingw-w64-x86_64-pkgconf mingw-w64-x86_64-rtmidi mingw-w64-x86_64-wavpack \
	mingw-w64-x86_64-wxwidgets3.2-msw mingw-w64-x86_64-yaml-cpp

# Download and install ASIO sdk
if [ ! -d /usr/local/asio-sdk ]; then
	DL_DIR=`mktemp -d -t asio.XXX`
	wget -O $DL_DIR/asiosdk.zip https://www.steinberg.net/asiosdk
	mkdir -p /usr/local
	unzip -o $DL_DIR/asiosdk.zip -d /usr/local/
	rm -rf $DL_DIR
	SDK_DIR=`ls -1d /usr/local/asiosdk* | tail -1`
	ln -sf `basename $SDK_DIR` /usr/local/asio-sdk
fi

# download vswhere to find MSVC
if [ ! -d /usr/local/bin/vswhere ]; then
	mkdir -p /usr/local/bin
	wget -O /usr/local/bin/vswhere https://github.com/microsoft/vswhere/releases/latest/download/vswhere.exe
fi

# download cv2pdb
if [ ! -d /usr/local/share/cv2pdb ]; then
	DL_DIR=`mktemp -d -t cv2pdb.XXX`
	wget -O $DL_DIR/cv2pdb-0.51.zip https://github.com/rainers/cv2pdb/releases/download/v0.51/cv2pdb-0.51.zip
	mkdir -p /usr/local/share/cv2pdb.tmp
	rm -rf /usr/local/share/cv2pdb.tmp/*
	unzip -d /usr/local/share/cv2pdb.tmp $DL_DIR/cv2pdb-0.51.zip
	rm -rf $DL_DIR
	mv /usr/local/share/cv2pdb.tmp /usr/local/share/cv2pdb
fi
