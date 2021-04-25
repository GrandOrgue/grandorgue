#!/bin/bash

set -e

sudo apt update

sudo DEBIAN_FRONTEND=noninteractive apt-get install -y cmake g++ pkg-config rpm file \
  libjack-dev libfftw3-dev zlib1g-dev libasound2-dev libwavpack-dev libudev-dev \
  libwxgtk3.0-gtk3-dev docbook-xsl xsltproc gettext po4a 
