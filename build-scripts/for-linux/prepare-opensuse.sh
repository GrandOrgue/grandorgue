#!/bin/sh

set -e
sudo zypper refresh
sudo zypper install -y cmake gcc-c++ gettext-tools libxslt-tools docbook-xsl-stylesheets po4a zip \
  libjack-devel fftw-devel zlib-devel wavpack-devel wxGTK3-devel alsa-devel libudev-devel ImageMagick
