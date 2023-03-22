#!/bin/sh

# makes the ubuntu system compatible 

set -e

UBUNTU_CODENAME=$(awk -F= '$1=="UBUNTU_CODENAME" {print $2;}'  /etc/os-release)

if [ -n "$UBUNTU_CODENAME" ]; then
  cat <<EOF > /tmp/sources.lst
deb [arch=amd64,i386] http://archive.ubuntu.com/ubuntu/ $UBUNTU_CODENAME main restricted universe multiverse
deb [arch=amd64,i386] http://archive.ubuntu.com/ubuntu/ $UBUNTU_CODENAME-updates main restricted universe multiverse
deb [arch=amd64,i386] http://archive.ubuntu.com/ubuntu/ $UBUNTU_CODENAME-backports main restricted universe multiverse
deb [arch=amd64,i386] http://security.ubuntu.com/ubuntu $UBUNTU_CODENAME-security main restricted universe multiverse
deb [arch=arm64,armhf] http://ports.ubuntu.com/ubuntu-ports/ $UBUNTU_CODENAME main restricted universe multiverse
deb [arch=arm64,armhf] http://ports.ubuntu.com/ubuntu-ports/ $UBUNTU_CODENAME-updates main restricted universe multiverse
deb [arch=arm64,armhf] http://ports.ubuntu.com/ubuntu-ports/ $UBUNTU_CODENAME-backports main restricted universe multiverse
deb [arch=arm64,armhf] http://ports.ubuntu.com/ubuntu-ports/ $UBUNTU_CODENAME-security main restricted universe multiverse
EOF
  sudo cp /tmp/sources.lst /etc/apt/sources.list
fi
