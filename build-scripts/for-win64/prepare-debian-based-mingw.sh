#!/bin/bash

set -e

# this gcc version interval has a bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=116159
GCC_BAD_VERSION_START=11
GCC_BAD_VERSION_END=14.3
GCC_MINGW_BUGFIX_VERSION=26.1.1

# determine an available mingw version
GCC_AVAILABLE_VERSION=$(apt-cache policy g++-mingw-w64 | sed 's/-/ /' | awk '/Candidate:/ { print $2; }' | sort | tail -n 1)

if [[ "$GCC_AVAILABLE_VERSION" < "$GCC_BAD_VERSION_START" ]] \
  || [[ "$GCC_BAD_VERSION_END" < "$GCC_AVAILABLE_VERSION" ]]; then
  sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
    g++-mingw-w64-x86-64-posix \
    gcc-mingw-w64-x86-64-posix \
    gcc-mingw-w64-x86-64-posix-runtime \
    gcc-mingw-w64-base
else
  #check installed version
  MINGW_INSALLED_VERSION=$(dpkg-query -W -f='${Version}' g++-mingw-w64-x86-64-posix 2>/dev/null | sed 's/.*+//' || true)

  if [[ "$MINGW_INSALLED_VERSION" != "$GCC_MINGW_BUGFIX_VERSION" ]]; then
    mkdir -p deb/mingw

    rm -rf deb/mingw
    wget -P deb/mingw\
      https://github.com/GrandOrgue/GccMingwW64/releases/download/${GCC_MINGW_BUGFIX_VERSION}/g++-mingw-w64-x86-64-posix_13.3.0-6ubuntu2.24.04+${GCC_MINGW_BUGFIX_VERSION}_amd64.deb \
      https://github.com/GrandOrgue/GccMingwW64/releases/download/${GCC_MINGW_BUGFIX_VERSION}/gcc-mingw-w64-x86-64-posix_13.3.0-6ubuntu2.24.04+${GCC_MINGW_BUGFIX_VERSION}_amd64.deb \
      https://github.com/GrandOrgue/GccMingwW64/releases/download/${GCC_MINGW_BUGFIX_VERSION}/gcc-mingw-w64-x86-64-posix-runtime_13.3.0-6ubuntu2.24.04+${GCC_MINGW_BUGFIX_VERSION}_amd64.deb \
      https://github.com/GrandOrgue/GccMingwW64/releases/download/${GCC_MINGW_BUGFIX_VERSION}/gcc-mingw-w64-base_13.3.0-6ubuntu2.24.04+${GCC_MINGW_BUGFIX_VERSION}_amd64.deb

    sudo DEBIAN_FRONTEND=noninteractive apt-get install -y ./deb/mingw/*.deb
  fi

fi
