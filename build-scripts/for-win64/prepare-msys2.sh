#!/bin/bash

# Installs the build dependencies for building GrandOrgue natively on Windows
# inside an MSYS2 MINGW64 shell. Meant to be run once via prepare-windows.cmd.

# $1 - wx package version: empty/auto or wx32 (see msys2-mingw64-packages.sh)

set -eu

BASE_DIR=$(dirname "$0")
WX_VER=${1:-}

# Toolchain and build-tool packages. RtAudio, RtMidi, PortAudio and
# ZitaConvolver are always built from the submodules/ sources (see
# USE_INTERNAL_* options in CMakeLists.txt), so no package for them is
# installed here.
TOOLCHAIN_PACKAGES=(
	base-devel git unzip wget zip
	mingw-w64-x86_64-cmake
	mingw-w64-x86_64-docbook-xsl
	mingw-w64-x86_64-gcc
	mingw-w64-x86_64-imagemagick
	mingw-w64-x86_64-libxslt
	mingw-w64-x86_64-librsvg
	mingw-w64-x86_64-nsis
	mingw-w64-x86_64-pkgconf
	mingw-w64-x86_64-zstd
)

# Library packages GrandOrgue links against - shared with
# prepare-debian-based.sh so both stay in sync.
source "$BASE_DIR/msys2-mingw64-packages.sh"
get_msys2_mingw64_lib_packages "$WX_VER"
LIB_PACKAGES=("${MSYS2_MINGW64_LIB_PACKAGES[@]/#/mingw-w64-x86_64-}")

pacman -S --needed --noconfirm "${TOOLCHAIN_PACKAGES[@]}" "${LIB_PACKAGES[@]}"

# download and install ASIO sdk
"$BASE_DIR/download-asio-sdk.sh"

# Download vswhere, used to locate the MSVC tools needed by cv2pdb
if [ ! -f /usr/local/bin/vswhere.exe ]; then
	mkdir -p /usr/local/bin
	wget -O /usr/local/bin/vswhere.exe https://github.com/microsoft/vswhere/releases/latest/download/vswhere.exe
fi

# download cv2pdb
"$BASE_DIR/download-cv2pdb.sh" /usr/local/share/cv2pdb
