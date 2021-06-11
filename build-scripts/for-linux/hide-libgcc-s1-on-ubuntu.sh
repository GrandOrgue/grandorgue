#!/bin/sh

# $1 - debian target architecture
TARGET_ARCH=$1

# old linux versions have libgcc1 instead of libgcc-s1
# prevent generating dependencies on libgcc-s1
if [ -f /var/lib/dpkg/info/libgcc-s1:$TARGET_ARCH.symbols ]; then
  sudo sh -c "echo \"libgcc_s 1 libgcc1 (>= 1:3.0)\" >>/etc/dpkg/shlibs.override"
  # overriding does not work while the symbol file exists
  sudo mv /var/lib/dpkg/info/libgcc-s1:$TARGET_ARCH.symbols /var/lib/dpkg/info/libgcc-s1:$TARGET_ARCH.symbols.old
fi 
