#!/bin/bash

# add an ppa repository for wxWidgets

# $1 - wx package name

set -e

WX_PACKAGE_NAME=$1

sudo apt-get update
if ! apt-cache show $WX_PACKAGE_NAME 2>/dev/null; then
  # the wx package is not found
  if [[ "$WX_PACKAGE_NAME" == "libwxgtk3.2-dev" ]]; then
    sudo DEBIAN_FRONTEND=noninteractive apt-get install -y software-properties-common
    echo "" | sudo DEBIAN_FRONTEND=noninteractive add-apt-repository ppa:sjr/wx32
  fi
fi
