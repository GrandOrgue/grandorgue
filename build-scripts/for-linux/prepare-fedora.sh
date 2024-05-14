#!/bin/sh

# $1 - wxWidgets package version: empty - libwxgtk3.2-dev or wx30 - libwxgtk3.0-gtk3-dev
# $2 - target architecture: ex arm64, amd64, armhf

WX_PKG_NAME=wxGTK-devel
[[ "$1" == "wx30" ]] && WX_PKG_NAME=wxGTK3-devel

set -e
sudo dnf install -y \
  cmake gcc-c++ make gettext docbook-style-xsl zip po4a ImageMagick rpm-build \
  pipewire-jack-audio-connection-kit-devel fftw-devel zlib-devel wavpack-devel \
  $WX_PKG_NAME alsa-lib-devel systemd-devel yaml-cpp-static dpkg-dev libcurl-devel
