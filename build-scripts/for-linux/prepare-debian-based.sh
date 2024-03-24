#!/bin/bash

# $1 - wxWidgets package version: empty - autoselect, wx30 - libwxgtk3.0-gtk3-dev or wx32 - libwxgtk3.2-dev
# $2 - target architecture: ex arm64, amd64, armhf

set -e

DIR=`dirname $0`

# calculate wx package name
case "$1" in
wx32)
  WX_PKG_NAME=libwxgtk3.2-dev
  ;;
wx30)
  WX_PKG_NAME=libwxgtk3.0-gtk3-dev
  ;;
*)
  sudo apt-get update
  # search for the last available version of libwxgtk
  WX_PKG_NAME=$(
    apt-cache pkgnames libwxgtk \
      | grep -e 'libwxgtk3.0-gtk3-dev\|libwxgtk[0-9].[0-9]-dev' \
      | sort \
      | tail -n 1 \
    )
  if [[ -z "$WX_PKG_NAME" ]]; then
    echo >&2 "No libwxgtk*-dev package available for this linux distribution"
    return 1 2>/dev/null || exit 1
  fi
esac

echo "wx package: $WX_PKG_NAME"

CURRENT_ARCH=$(dpkg --print-architecture)
TARGET_ARCH="${2:-$CURRENT_ARCH}"

OS_DISTR=$(awk -F= '$1=="ID" {print $2;}' /etc/os-release)
[[ "$OS_DISTR" == "ubuntu" ]] && $DIR/prepare-ubuntu-wx-repo.bash $WX_PKG_NAME

if [[ "$TARGET_ARCH" == "$CURRENT_ARCH" ]]; then
  GCC_SUFFIX=""
  sudo apt update
else
  sudo dpkg --add-architecture "$TARGET_ARCH"

  # ubuntu has different urls for different architectures
  [[ "$OS_DISTR" == "ubuntu" ]] && $DIR/prepare-ubuntu-multiarch-repos.sh
  sudo apt update

  # remove an odd version of packages that prevents installing same packages for foreign arch
  $DIR/prepare-debian-based-align-libs.sh $TARGET_ARCH

  sudo DEBIAN_FRONTEND=noninteractive apt-get install -y dpkg-dev
  GCC_SUFFIX=-$(dpkg-architecture -A $TARGET_ARCH -q DEB_TARGET_MULTIARCH)
fi

# it is necessary for future installing wx
sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
  libcups2:$TARGET_ARCH

sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
  cmake \
  docbook-xsl \
  dpkg-dev \
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
  ${WX_PKG_NAME}:$TARGET_ARCH \
  libyaml-cpp-dev:$TARGET_ARCH \
  zlib1g-dev:$TARGET_ARCH \
  libcurl4-openssl-dev:$TARGET_ARCH

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

# some ppas for wxWidgets give very high dependency version that prevents
# installing on other systems. Remove the version
if dpkg -s libwxgtk3.2-dev 2>/dev/null && ! grep -q libwx /etc/dpkg/shlibs.override; then
  cut -d " " -f 1-3 /var/lib/dpkg/info/libwx*3.2*.shlibs | sudo sh -c "cat >>/etc/dpkg/shlibs.override"
fi