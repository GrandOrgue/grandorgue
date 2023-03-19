#!/bin/sh

set -e
sudo dnf install -y \
  cmake gcc-c++ make gettext docbook-style-xsl zip po4a ImageMagick rpm-build \
  pipewire-jack-audio-connection-kit-devel fftw-devel zlib-devel wavpack-devel \
  wxGTK3-devel alsa-lib-devel systemd-devel yaml-cpp-static
