#!/usr/bin/sh

DIR=`dirname $0`

svn log | awk -f $DIR/svn-changelog.awk
