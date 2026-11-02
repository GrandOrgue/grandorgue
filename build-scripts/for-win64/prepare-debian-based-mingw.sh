#!/bin/bash

set -e

# Debian 13 (trixie) ships the UCRT-targeting mingw-w64 cross toolchain
# (x86_64-w64-mingw32ucrt-gcc) together with mingw-w64 27. Older Ubuntu
# releases only provide the msvcrt-targeting one and pin an outdated
# mingw-w64 that needed a patched GCC build; neither is needed any more.
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
  g++-mingw-w64-ucrt64 \
  gcc-mingw-w64-ucrt64
