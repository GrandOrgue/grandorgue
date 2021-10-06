#!/bin/bash

CHANGELOG=CHANGELOG.md
VERSION_TXT=version.txt

# make the tag from the version number.
# if the version does not contain a build version then add the default build version
assure_tag()
{
  TAG=$1
  SUFFIX_DEFAULT=$2

  [[ -z "$(echo $TAG | cut -s -d- -f2)" ]] && TAG=${TAG}-${SUFFIX_DEFAULT}
  echo $TAG
}

set -x

GIT_TAG=`git describe --tags | cut -d- -f1-2`
NEW_TAG=
CHANGELOG_HEAD=$(head -n 1 $CHANGELOG)
CHANGELOG_TAG=

if [[ "$(echo $CHANGELOG_HEAD | cut -d\  -f1)" == "#" ]]; then
  echo release
  # release
  CHANGELOG_TAG=$(assure_tag $(echo $CHANGELOG_HEAD | cut -d\  -f2) 1)
fi
if [[ "$CHANGELOG_TAG" > "$GIT_TAG" ]]; then
  # a new release. Push the release tag
  NEW_TAG=$CHANGELOG_TAG
else
  # No release. May be we are preparing for a new release in the future
  FUTURE_RELEASE_TAG=$(assure_tag $(cat $VERSION_TXT) 0)
  [[ "$FUTURE_RELEASE_TAG" > "$GIT_TAG" ]] && NEW_TAG=$FUTURE_RELEASE_TAG
fi
echo "NEW_TAG=$NEW_TAG"
