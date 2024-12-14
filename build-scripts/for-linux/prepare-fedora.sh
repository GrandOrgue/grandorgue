#!/bin/sh

# $1 - wxWidgets package version: empty - libwxgtk3.2-dev or wx30 - libwxgtk3.0-gtk3-dev
# $2 - target architecture: ex arm64, amd64, armhf

DIR=`dirname $0`

set -e

. $DIR/prepare-parse-prms.bash

WX_PKG_NAME=wxGTK-devel
[[ "WX_VER" == "wx30" ]] && WX_PKG_NAME=wxGTK3-devel

OPTIONAL_PKGS=""
[[ "$INSTALL_TESTS" == "tests" ]] && OPTIONAL_PKGS="$OPTIONAL_PKGS gcovr"

sudo dnf install -y \
  cmake gcc-c++ make gettext docbook-style-xsl zip po4a ImageMagick rpm-build $OPTIONAL_PKGS \
  pipewire-jack-audio-connection-kit-devel fftw-devel zlib-devel wavpack-devel \
  $WX_PKG_NAME alsa-lib-devel systemd-devel yaml-cpp-static dpkg-dev libcurl-devel

# install cpptrace
[[ "$INSTALL_TESTS" == "tests" ]] && $DIR/prepare-cpptrace.bash
