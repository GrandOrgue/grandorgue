#!/bin/bash

# Installs the build dependencies for building GrandOrgue natively on Windows
# inside an MSYS2 UCRT64 shell. Meant to be run once via prepare-windows.cmd.

# Accepts, in any order:
#   auto (default) - wxWidgets version. Only auto is available: it picks
#     wxWidgets 3.3, the same series that the win64 cross-build links against
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
	auto)
		WX_VER=$prm
		;;
	wx32)
		echo "Unsupported wx version $prm: only auto is available" >&2
		exit 1
		;;
	tests | notests)
		INSTALL_TESTS=$prm
		;;
	asan | noasan)
		INSTALL_ASAN=$prm
		;;
	*)
		echo "Unknown parameter $prm" >&2
		echo "Usage: $(basename "$0") [auto] [tests | notests] [asan | noasan]" >&2
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

WX_PACKAGES=(
	mingw-w64-ucrt-x86_64-wxwidgets3.3-common
	mingw-w64-ucrt-x86_64-wxwidgets3.3-common-libs
	mingw-w64-ucrt-x86_64-wxwidgets3.3-msw
	mingw-w64-ucrt-x86_64-wxwidgets3.3-msw-libs
)

# wxWidgets 3.2 ships its own /ucrt64/bin/wx-config, which conflicts with the
# 3.3 one, so it has to be removed before 3.3 can be installed
OLD_WX_PACKAGES=$(pacman -Qq 2>/dev/null | grep '^mingw-w64-ucrt-x86_64-wxwidgets3\.2-' || true)
if [[ -n "$OLD_WX_PACKAGES" ]]; then
	pacman -R --noconfirm $OLD_WX_PACKAGES
fi

pacman -S --needed --noconfirm \
	"${TOOLCHAIN_PACKAGES[@]}" "${LIB_PACKAGES[@]}" "${WX_PACKAGES[@]}"

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
