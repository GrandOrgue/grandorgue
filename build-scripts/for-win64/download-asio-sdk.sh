#!/bin/bash

# Downloads and installs the ASIO SDK into /usr/local/asio-sdk, unless
# already present. Shared between prepare-debian-based.sh (Linux
# cross-compile, needs sudo to write to /usr/local) and prepare-msys2.sh
# (native/MSYS2 build, /usr/local is inside the MSYS2 root and normally
# user-writable).

set -e

if [ -w /usr/local ] || [ "$(id -u)" = "0" ]; then
	SUDO=
else
	SUDO=sudo
fi

if [ ! -d /usr/local/asio-sdk ]; then
	DL_DIR=$(mktemp -d -t asio.XXX)
	wget -O "$DL_DIR/asiosdk.zip" https://www.steinberg.net/asiosdk
	$SUDO mkdir -p /usr/local
	$SUDO unzip -o "$DL_DIR/asiosdk.zip" -d /usr/local/
	rm -rf "$DL_DIR"
	SDK_DIR=$(ls -1d /usr/local/asiosdk* /usr/local/ASIOSDK* 2>/dev/null | tail -1)
	$SUDO ln -sf "$(basename "$SDK_DIR")" /usr/local/asio-sdk
fi
