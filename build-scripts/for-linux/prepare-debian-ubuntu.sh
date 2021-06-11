#!/bin/bash

set -e

sudo apt update

sudo DEBIAN_FRONTEND=noninteractive apt-get install -y cmake g++ pkg-config rpm file \
  libjack-dev libfftw3-dev zlib1g-dev libasound2-dev libwavpack-dev libudev-dev \
  libwxgtk3.0-gtk3-dev docbook-xsl xsltproc gettext po4a

# old linux versions have libgcc1 instead of libgcc-s1
# prevent generating dependencies on libgcc-s1
if [ -f /var/lib/dpkg/info/libgcc-s1:amd64.symbols ]; then
  sudo sh -c "echo \"libgcc_s 1 libgcc1 (>= 1:3.0)\" >>/etc/dpkg/shlibs.override"
  # overriding does not work while the symbol file exists
  sudo mv /var/lib/dpkg/info/libgcc-s1:amd64.symbols /var/lib/dpkg/info/libgcc-s1:amd64.symbols.old
fi 
