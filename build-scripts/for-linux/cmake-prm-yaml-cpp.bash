#!/bin/bash

# Find the yaml-cpp-static cmake path and print the -Dyaml-cpp_DIR parameter
# $1 - target deb architecture. Default - current

set -e

TARGET_ARCH=${1:$(dpkg --print-architecture)}

MULTIARCH=$(dpkg-architecture -q DEB_TARGET_MULTIARCH -A $TARGET_ARCH)
if [[ -d /usr/lib/$MULTIARCH/cmake/yaml-cpp-static ]]; then
  CMAKE_PRM="-Dyaml-cpp_DIR=/usr/lib/$MULTIARCH/cmake/yaml-cpp-static"
elif [[ -d /usr/lib64/cmake/yaml-cpp-static ]]; then
  CMAKE_PRM="-Dyaml-cpp_DIR=/usr/lib64/cmake/yaml-cpp-static"
else
  CMAKE_PRM=""
fi
echo $CMAKE_PRM
