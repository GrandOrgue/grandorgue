#!/bin/bash

WX_VER=auto
INSTALL_TESTS=notests

# parse parameters
while [[ $# -gt 0 ]]; do
  case $1 in
    wx30 | wx32)
      WX_VER=$1
      ;;
    tests | notests)
      INSTALL_TESTS=$1
      ;;
    *)
      echo "Unknown parameter $1" >&2
      echo "Usage: $(basename $0) [wx30 | wx32] [tests | notests]"
      exit 1
      ;;
  esac
  shift # past a parameter
done
