# Computes the names (without the mingw-w64-ucrt-x86_64- prefix) of the MSYS2
# mingw-w64 library packages GrandOrgue links against on Windows, into the
# MSYS2_LIB_PACKAGES array. Shared between prepare-debian-based.sh (fetched via
# msys2dl and converted to .deb, for ABI compatibility with the cross-compiling
# mingw toolchain) and prepare-msys2.sh (installed directly via pacman for a
# native build), so both stay in sync.

MSYS2_LIB_PACKAGES=(curl-winssl fftw jack2 wavpack yaml-cpp wxwidgets3.3-msw)
