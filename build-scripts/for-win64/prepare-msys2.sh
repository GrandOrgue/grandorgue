#!/bin/bash

# Installs the build dependencies for building GrandOrgue natively on Windows
# inside an MSYS2 UCRT64 shell. Meant to be run once via prepare-windows.cmd.

# Accepts, in any order:
#   wx33 or auto (default) - wxWidgets version. Only 3.3 is available for the
#     UCRT64 environment
#   tests or notests (default) - also install gcovr for code coverage reports
#   asan or noasan (default) - AddressSanitizer support; bundled with the
#     mingw-w64-ucrt-x86_64-gcc package, so this only accepts the keyword for
#     parity with prepare-debian-based.sh - nothing extra is installed

set -eu

BASE_DIR=$(dirname "$0")
WX_VER=auto
INSTALL_TESTS=notests
INSTALL_ASAN=noasan

for prm in "$@"; do
	case "$prm" in
	wx33 | auto)
		WX_VER=$prm
		;;
	tests | notests)
		INSTALL_TESTS=$prm
		;;
	asan | noasan)
		INSTALL_ASAN=$prm
		;;
	*)
		echo "Unknown parameter $prm" >&2
		echo "Usage: $(basename "$0") [wx33 | auto] [tests | notests] [asan | noasan]" >&2
		exit 1
		;;
	esac
done

# Toolchain and build-tool packages. RtAudio, RtMidi, PortAudio and
# ZitaConvolver are always built from the submodules/ sources (see
# USE_INTERNAL_* options in CMakeLists.txt), so no package for them is
# installed here.
TOOLCHAIN_PACKAGES=(
	base-devel git unzip wget zip
	mingw-w64-ucrt-x86_64-cmake
	mingw-w64-ucrt-x86_64-docbook-xsl
	mingw-w64-ucrt-x86_64-gcc
	mingw-w64-ucrt-x86_64-imagemagick
	mingw-w64-ucrt-x86_64-libxslt
	mingw-w64-ucrt-x86_64-librsvg
	mingw-w64-ucrt-x86_64-nsis
	mingw-w64-ucrt-x86_64-pkgconf
	mingw-w64-ucrt-x86_64-zstd
)

# Library packages GrandOrgue links against - shared with
# prepare-debian-based.sh so both stay in sync.
source "$BASE_DIR/msys2-packages.sh"
LIB_PACKAGES=("${MSYS2_LIB_PACKAGES[@]/#/mingw-w64-ucrt-x86_64-}")

pacman -S --needed --noconfirm \
	"${TOOLCHAIN_PACKAGES[@]}" "${LIB_PACKAGES[@]}"

if [[ "$INSTALL_TESTS" == "tests" ]]; then
	pacman -S --needed --noconfirm mingw-w64-ucrt-x86_64-gcovr
fi

# download and install ASIO sdk
"$BASE_DIR/download-asio-sdk.sh"

# Download vswhere, used to locate the MSVC tools needed by cv2pdb
if [ ! -f /usr/local/bin/vswhere.exe ]; then
	mkdir -p /usr/local/bin
	wget -O /usr/local/bin/vswhere.exe https://github.com/microsoft/vswhere/releases/latest/download/vswhere.exe
fi

# download cv2pdb
"$BASE_DIR/download-cv2pdb.sh" /usr/local/share/cv2pdb
