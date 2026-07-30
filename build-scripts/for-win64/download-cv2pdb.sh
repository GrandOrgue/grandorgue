#!/bin/bash

# Downloads and installs cv2pdb (converts mingw debug info into a .pdb file)
# into the directory given as $1, unless already present. Shared between
# prepare-debian-based.sh (Linux cross-compile, installs it for use under
# wine, needs sudo to write to /usr/local) and prepare-msys2.sh
# (native/MSYS2 build, /usr/local is inside the MSYS2 root and normally
# user-writable).

set -e

TARGET_DIR=$1

if [ -w "$(dirname "$TARGET_DIR")" ] || [ "$(id -u)" = "0" ]; then
	SUDO=
else
	SUDO=sudo
fi

if [ ! -d "$TARGET_DIR" ]; then
	DL_DIR=$(mktemp -d -t cv2pdb.XXX)
	wget -O "$DL_DIR/cv2pdb-0.51.zip" https://github.com/rainers/cv2pdb/releases/download/v0.51/cv2pdb-0.51.zip
	$SUDO mkdir -p "$TARGET_DIR.tmp"
	$SUDO rm -rf "${TARGET_DIR:?}.tmp"/*
	$SUDO unzip -d "$TARGET_DIR.tmp" "$DL_DIR/cv2pdb-0.51.zip"
	rm -rf "$DL_DIR"
	$SUDO mv "$TARGET_DIR.tmp" "$TARGET_DIR"
fi
