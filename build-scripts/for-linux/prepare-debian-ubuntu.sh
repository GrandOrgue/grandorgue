#!/bin/sh

set -e
sudo apt update
sudo DEBIAN_FRONTEND=noninteractive apt install -y cmake g++ pkg-config \
  libjack-dev libfftw3-dev zlib1g-dev libasound2-dev libwavpack-dev libwxgtk3.0-dev libudev-dev \
  docbook-xsl xsltproc gettext po4a  
