#!/bin/bash

#rem $1 - target architecture: ex arm64, amd64, armhf or i386

set -e

DIR=`dirname $0`

TARGET_ARCH="${1:-$(dpkg --print-architecture)}"

# remove an odd version of libpcre2-8-0 and libgd3 that prevents installing
# packages for foreign archs
LIBPCRE_VERSION=`apt-cache policy libpcre2-8-0:$TARGET_ARCH | awk '/Candidate:/ { print $2; }'`
LIBDG3_VERSION=`apt-cache policy libgd3:$TARGET_ARCH | awk '/Candidate:/ { print $2; }'`
sudo DEBIAN_FRONTEND=noninteractive apt-get --allow-downgrades -y install \
  libpcre2-8-0=$LIBPCRE_VERSION libgd3=$LIBDG3_VERSION
