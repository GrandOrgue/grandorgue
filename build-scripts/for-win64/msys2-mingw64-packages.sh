# Computes the names (without the mingw-w64-x86_64- prefix) of the MSYS2
# mingw64 library packages GrandOrgue links against on Windows, into the
# MSYS2_MINGW64_LIB_PACKAGES array. Shared between prepare-debian-based.sh
# (fetched via msys2dl and converted to .deb, for ABI compatibility with the
# cross-compiling mingw toolchain) and prepare-msys2.sh (installed directly
# via pacman for a native build), so both stay in sync.
#
# MSYS2 is phasing out the classic MINGW64 environment and has already removed
# all wxWidgets packages from its package database. So wxWidgets is not part
# of MSYS2_MINGW64_LIB_PACKAGES:
#  - the Linux cross-build installs wxWidgets 3.3.4 built by
#    https://github.com/GrandOrgue/WxWidgetsCross
#    (see prepare-debian-based.sh);
#  - the native MSYS2 build installs the last MINGW64 packages of wxWidgets
#    3.2.11-1 directly from the MSYS2 repository by pacman -U, using
#    MSYS2_MINGW64_WX_PACKAGE_URLS (see prepare-msys2.sh).
# Both are temporary until the migration to UCRT64
# (https://github.com/GrandOrgue/grandorgue/issues/2671).

MSYS2_MINGW64_LIB_PACKAGES=(curl-winssl fftw jack2 wavpack yaml-cpp)

# The pinned wxWidgets MINGW64 packages, only for the native build
MSYS2_MINGW64_WX_PACKAGE_URLS=(
	https://repo.msys2.org/mingw/mingw64/mingw-w64-x86_64-wxwidgets3.2-common-3.2.11-1-any.pkg.tar.zst
	https://repo.msys2.org/mingw/mingw64/mingw-w64-x86_64-wxwidgets3.2-common-libs-3.2.11-1-any.pkg.tar.zst
	https://repo.msys2.org/mingw/mingw64/mingw-w64-x86_64-wxwidgets3.2-msw-3.2.11-1-any.pkg.tar.zst
	https://repo.msys2.org/mingw/mingw64/mingw-w64-x86_64-wxwidgets3.2-msw-libs-3.2.11-1-any.pkg.tar.zst
)
