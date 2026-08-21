#!/bin/bash

# Installs the build dependencies for building GrandOrgue natively on Windows
# inside an MSYS2 MINGW64 shell. Meant to be run once via prepare-windows.cmd.

# Accepts, in any order:
#   wx32 or auto (default) - wxWidgets package version (see
#     msys2-mingw64-packages.sh)
#   tests or notests (default) - also install gcovr for code coverage reports
#   asan or noasan (default) - AddressSanitizer support; bundled with the
#     mingw-w64-x86_64-gcc package, so this only accepts the keyword for
#     parity with prepare-debian-based.sh - nothing extra is installed

# gcovr has no package of its own for the classic MINGW64 environment, and
# pip-installing it there fails to build its lxml dependency from source
# (missing -lzlib). mingw-w64-ucrt-x86_64-gcovr is a prebuilt, self-contained
# alternative from the ucrt64 environment; it's exposed on PATH via a small
# wrapper script since /ucrt64/bin isn't part of the MINGW64 shell's PATH.

set -eu

BASE_DIR=$(dirname "$0")
WX_VER=auto
INSTALL_TESTS=notests
INSTALL_ASAN=noasan

for prm in "$@"; do
	case "$prm" in
	wx32 | auto)
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
		echo "Usage: $(basename "$0") [wx32 | auto] [tests | notests] [asan | noasan]" >&2
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

if [[ "$INSTALL_TESTS" == "tests" ]]; then
	pacman -S --needed --noconfirm mingw-w64-ucrt-x86_64-gcovr

	if [ ! -f /usr/local/bin/gcovr ]; then
		mkdir -p /usr/local/bin
		printf '#!/bin/bash\nexec /ucrt64/bin/gcovr.exe "$@"\n' >/usr/local/bin/gcovr
		chmod +x /usr/local/bin/gcovr
	fi
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
