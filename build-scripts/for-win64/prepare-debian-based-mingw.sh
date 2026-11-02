#!/bin/bash

set -e

# Debian 13 (trixie) ships the UCRT-targeting mingw-w64 cross toolchain
# (x86_64-w64-mingw32ucrt-gcc, GCC 14). Older Ubuntu releases only provide the
# msvcrt-targeting one, built from the gcc-mingw-w64 26.x packaging whose
# libstdc++ lacked the PR116159 fix, which is why a rebuilt toolchain used to
# be downloaded from GccMingwW64. GCC 14 already contains that fix, so the
# packaged toolchain is used as is.
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
  g++-mingw-w64-ucrt64 \
  gcc-mingw-w64-ucrt64
