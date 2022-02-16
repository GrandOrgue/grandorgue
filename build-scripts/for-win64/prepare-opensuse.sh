#!/bin/sh

set -e

sudo zypper install -y rpm-build gcc-c++ cmake gettext-tools docbook-xsl-stylesheets po4a libxslt-tools zip
sudo zypper addrepo http://download.opensuse.org/repositories/windows:mingw:win32/openSUSE_Tumbleweed/windows:mingw:win32.repo
sudo zypper addrepo http://download.opensuse.org/repositories/windows:mingw:win64/openSUSE_Tumbleweed/windows:mingw:win64.repo
sudo zypper addrepo http://download.opensuse.org/repositories/home:e9925248:mingw/openSUSE_Tumbleweed/home:e9925248:mingw.repo
sudo zypper modifyrepo -p 10 home_e9925248_mingw
sudo zypper --gpg-auto-import-keys refresh
sudo zypper install -y mingw32-cross-nsis mingw64-cross-binutils mingw64-cross-gcc mingw64-cross-gcc-c++ mingw64-cross-pkg-config \
    mingw64-fftw3 mingw64-fftw3-devel mingw64-filesystem mingw64-jack mingw64-jack-devel mingw64-libgnurx-devel mingw64-libgnurx0 \
    mingw64-wavpack mingw64-wavpack-devel mingw64-wxWidgets-3_0-devel mingw64-wxWidgets-3_0-lang mingw64-zlib-devel mingw64-zlib1 \
    ImageMagick zip

