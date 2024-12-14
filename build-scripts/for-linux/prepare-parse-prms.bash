#!/bin/bash

INSTALL_TESTS=notests
TARGET_CPU=auto
WX_VER=auto

# parse parameters
while [[ $# -gt 0 ]]; do
  if [[ -n "$1" ]]; then
    case $1 in
      amd64 | armhf | arm64)
	TARGET_CPU=$1
	;;
      tests | notests)
	INSTALL_TESTS=$1
	;;
      wx30 | wx32)
	WX_VER=$1
	;;
      *)
	echo "Unknown parameter $1" >&2
	echo "Usage: $(basename $0) [wx30 | wx32] [tests | notests]"
	exit 1
	;;
    esac
  fi
  shift # past a parameter
done
