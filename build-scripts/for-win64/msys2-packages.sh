# Computes the names (without the mingw-w64-<env>-x86_64- prefix) of the MSYS2
# mingw-w64 library packages GrandOrgue links against on Windows, into the
# MSYS2_LIB_PACKAGES array. Shared between prepare-debian-based.sh (fetched via
# msys2dl and converted to .deb, for ABI compatibility with the cross-compiling
# mingw toolchain) and prepare-msys2.sh (installed directly via pacman for a
# native build), so both stay in sync.
#
# wxWidgets is not part of this list because the two consumers obtain it
# differently: the Linux cross-build installs its own build of wxWidgets 3.3.4
# from https://github.com/GrandOrgue/WxWidgetsCross (see
# prepare-debian-based.sh), while the native MSYS2 build installs the
# wxwidgets3.2 packages from the MSYS2 repository (see prepare-msys2.sh).

MSYS2_LIB_PACKAGES=(curl-winssl fftw jack2 wavpack yaml-cpp)
