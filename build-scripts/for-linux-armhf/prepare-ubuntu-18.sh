#!/bin/bash

set -e

sudo dpkg --add-architecture armhf
cat <<EOF > /tmp/sources.lst
deb [arch=amd64] http://archive.ubuntu.com/ubuntu/ bionic main restricted universe multiverse
deb [arch=amd64] http://archive.ubuntu.com/ubuntu/ bionic-updates main restricted universe multiverse
deb [arch=amd64] http://archive.ubuntu.com/ubuntu/ bionic-backports main restricted universe multiverse
deb [arch=amd64] http://security.ubuntu.com/ubuntu bionic-security main restricted universe multiverse
deb [arch=arm64,armhf] http://ports.ubuntu.com/ubuntu-ports/ bionic main restricted universe multiverse
deb [arch=arm64,armhf] http://ports.ubuntu.com/ubuntu-ports/ bionic-updates main restricted universe multiverse
deb [arch=arm64,armhf] http://ports.ubuntu.com/ubuntu-ports/ bionic-backports main restricted universe multiverse
deb [arch=arm64,armhf] http://ports.ubuntu.com/ubuntu-ports/ bionic-security main restricted universe multiverse
EOF
sudo cp /tmp/sources.lst /etc/apt/sources.list
sudo apt update

sudo DEBIAN_FRONTEND=noninteractive apt-get install -y cmake g++ pkg-config rpm file \
  g++-arm-linux-gnueabihf gcc-arm-linux-gnueabihf \
  libjack-dev:armhf libfftw3-dev:armhf zlib1g-dev:armhf libasound2-dev:armhf libwavpack-dev:armhf libudev-dev:armhf \
  libwxgtk3.0-gtk3-dev:armhf docbook-xsl xsltproc gettext po4a imagemagick

DIR=`dirname $0`
$DIR/../for-linux/hide-libgcc-s1-on-ubuntu.sh armhf
