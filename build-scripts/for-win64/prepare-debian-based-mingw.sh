#!/bin/bash

set -e

# Ubuntu 26.10 and Debian 14 (forky) ship the UCRT-targeting mingw-w64 cross
# toolchain from gcc-mingw-w64 29 (x86_64-w64-mingw32ucrt-gcc, GCC 16).
# GCC 16 is required: the libstdc++ runtime downloaded from MSYS2 is GCC 16,
# and with the headers of an older GCC (e.g. 14 in Debian 13) the executables
# import __emutls_v._ZSt11__once_call, which the GCC 16 libstdc++-6.dll does
# not export (GCC commit r16-6251), so they do not start on Windows.
# Older releases provide either the msvcrt-targeting toolchain or GCC < 16.
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
  g++-mingw-w64-ucrt64 \
  gcc-mingw-w64-ucrt64
