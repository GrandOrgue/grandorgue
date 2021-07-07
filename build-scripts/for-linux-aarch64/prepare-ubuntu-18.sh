#!/bin/bash

set -e

sudo dpkg --add-architecture arm64
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
  g++-aarch64-linux-gnu gcc-aarch64-linux-gnu \
  libjack-dev:arm64 libfftw3-dev:arm64 zlib1g-dev:arm64 libasound2-dev:arm64 libwavpack-dev:arm64 libudev-dev:arm64 \
  libwxgtk3.0-gtk3-dev:arm64 docbook-xsl xsltproc gettext po4a imagemagick

DIR=`dirname $0`
$DIR/../for-linux/hide-libgcc-s1-on-ubuntu.sh arm64
