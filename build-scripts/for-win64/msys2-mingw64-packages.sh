# Computes the names (without the mingw-w64-x86_64- prefix) of the MSYS2
# mingw64 library packages GrandOrgue links against on Windows, into the
# MSYS2_MINGW64_LIB_PACKAGES array. Shared between prepare-debian-based.sh
# (fetched via msys2dl and converted to .deb, for ABI compatibility with the
# cross-compiling mingw toolchain) and prepare-msys2.sh (installed directly
# via pacman for a native build), so both stay in sync.
#
# $1 - wx package version: empty/auto or wx32 - wxwidgets3.2-msw.
#      A wxwidgets3.3-msw package also exists in MSYS2, but only for the
#      mingw-w64-ucrt-x86_64/mingw-w64-clang-x86_64 environments, not the
#      classic mingw-w64-x86_64 (MINGW64) environment these scripts target
#      (msys2_shell.cmd -mingw64), so it isn't offered as an option here.

get_msys2_mingw64_lib_packages() {
	local wx_pkg

	case "${1:-auto}" in
	auto | wx32)
		wx_pkg=wxwidgets3.2-msw
		;;
	*)
		echo "Unsupported wx version $1: only wx32 (wxwidgets3.2-msw) is available for the MINGW64 environment" >&2
		return 1
		;;
	esac

	MSYS2_MINGW64_LIB_PACKAGES=(curl-winssl fftw jack2 wavpack yaml-cpp "$wx_pkg")
}
