#!/bin/bash

#rem $1 - target architecture: ex arm64, amd64, armhf

set -e

DIR=`dirname $0`

CURRENT_ARCH=$(dpkg --print-architecture)
TARGET_ARCH="${1:-$CURRENT_ARCH}"

if [[ "$TARGET_ARCH" == "$CURRENT_ARCH" ]]; then
  GCC_SUFFIX=""
  sudo apt update
else
  sudo dpkg --add-architecture "$TARGET_ARCH"

  OS_DISTR=$(awk -F= '$1=="ID" {print $2;}'  /etc/os-release)
  # ubuntu has different urls for different architectures
  [[ "$OS_DISTR" == "ubuntu" ]] && $DIR/prepare-ubuntu-multiarch-repos.sh
  sudo apt update

  # remove an odd version of packages that prevents installing same packages for foreign arch
  $DIR/prepare-debian-based-align-libs.sh $TARGET_ARCH

  sudo DEBIAN_FRONTEND=noninteractive apt-get install -y dpkg-dev
  GCC_SUFFIX=-$(dpkg-architecture -A $TARGET_ARCH -q DEB_TARGET_MULTIARCH)
fi

sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
  cmake \
  docbook-xsl \
  file \
  gettext \
  imagemagick \
  pkg-config \
  po4a \
  rpm \
  wget \
  xsltproc \
  g++$GCC_SUFFIX \
  libasound2-dev:$TARGET_ARCH \
  libfftw3-dev:$TARGET_ARCH \
  libjack-dev:$TARGET_ARCH \
  libudev-dev:$TARGET_ARCH \
  libwavpack-dev:$TARGET_ARCH \
  libwxgtk3.0-gtk3-dev:$TARGET_ARCH \
  libyaml-cpp-dev:$TARGET_ARCH \
  zlib1g-dev:$TARGET_ARCH

# download and install additional packages
mkdir -p deb
pushd deb

wget \
  https://github.com/GrandOrgue/YamlCppAdd/releases/download/0.6.2-5.go/libyaml-cpp-static_0.6.2-5.go_all.deb

sudo apt-get install -y \
  ./libyaml-cpp-static_0.6.2-5.go_all.deb
popd

# old linux versions have libgcc1 instead of libgcc-s1
# prevent generating dependencies on libgcc-s1
if [ -f /var/lib/dpkg/info/libgcc-s1:$TARGET_ARCH.symbols ]; then
  sudo sh -c "echo \"libgcc_s 1 libgcc1 (>= 1:3.0)\" >>/etc/dpkg/shlibs.override"
  # overriding does not work while the symbol file exists
  sudo mv /var/lib/dpkg/info/libgcc-s1:$TARGET_ARCH.symbols /var/lib/dpkg/info/libgcc-s1:$TARGET_ARCH.symbols.old
fi
