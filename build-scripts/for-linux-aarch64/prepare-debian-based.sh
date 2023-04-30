#!/bin/bash

# $1 - wxWidgets package version: empty - libwxgtk3.2-dev or wx30 - libwxgtk3.0-gtk3-dev

set -e

DIR=`dirname $0`
$DIR/../for-linux/prepare-debian-based.sh "$1" arm64
