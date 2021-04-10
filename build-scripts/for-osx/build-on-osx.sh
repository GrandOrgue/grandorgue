#!/bin/bash

# $1 - Revision
# $2 - Go source Dir. If not set then relative to the script dir

set -e

if [[ -n "$1" ]]; then
	REVISION_PRM=-DVERSION_REVISION=$1
else
	REVISION_PRM=
fi

if [[ -n "$2" ]]; then
	SRC_DIR=$2
else
	SRC_DIR=$(dirname $0)/../..
fi

PARALLEL_PRMS="-j4"

mkdir -p build-osx
pushd build-osx

rm -rf *
export LANG=C

OS_PRMS="-DDOCBOOK_DIR=/usr/local/opt/docbook-xsl/docbook-xsl"
GO_PRMS="-DCMAKE_BUILD_TYPE=Release $REVISION_PRM"
cmake -G "Unix Makefiles" $OS_PRMS $GO_PRMS $SRC_DIR
make -k $PARALLEL_PRMS VERBOSE=1 package

popd

