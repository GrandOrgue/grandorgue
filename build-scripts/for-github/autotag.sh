#!/bin/bash

CHANGELOG=CHANGELOG.md
VERSION_TXT=version.txt
RELEASE_NOTES=RELNOTES.md

# make the tag from the version number.
# if the version does not contain a build version then add the default build version
assure_full_version()
{
  TAG=$1
  BUILD_DEFAULT=$2

  [[ -z "$(echo $TAG | cut -s -d- -f2)" ]] && TAG=${TAG}-${BUILD_DEFAULT}
  echo $TAG
}

git fetch --prune --unshallow --tags
GIT_TAG=`git describe --tags | cut -d- -f1-2`
NEW_TAG=
CHANGELOG_HEAD=$(head -n 1 $CHANGELOG)
CHANGELOG_TAG=

if [[ "$(echo $CHANGELOG_HEAD | cut -d\  -f1)" == "#" ]]; then
  echo release
  # release
  CHANGELOG_TAG=$(assure_full_version $(echo $CHANGELOG_HEAD | cut -d\  -f2) 1)
fi
if [[ -n "$CHANGELOG_TAG" ]] && [[ ! "$CHANGELOG_TAG" < "$GIT_TAG" ]]; then
  # a new release. Push the release tag and make the relnotes
  NEW_TAG=$CHANGELOG_TAG
  sed '0,/^#/d;/^#/Q' CHANGELOG.md >$RELEASE_NOTES
else
  # No release. May be we are preparing for a new release in the future
  FUTURE_RELEASE_TAG=$(assure_full_version $(cat $VERSION_TXT) 0)
  [[ "$FUTURE_RELEASE_TAG" > "$GIT_TAG" ]] && NEW_TAG=$FUTURE_RELEASE_TAG
fi
echo "NEW_TAG=$NEW_TAG"
if [[ -n "$NEW_TAG" ]]; then
  # If moving an existing tag, check the release is not already published.
  # If the release is a draft or does not exist yet, push the tag as usual.
  if [[ "$NEW_TAG" == "$GIT_TAG" ]] && [[ -n "$GH_TOKEN" ]]; then
    IS_DRAFT=$(gh release view "$NEW_TAG" --json isDraft --jq '.isDraft' 2>/dev/null || echo "notfound")
    if [[ "$IS_DRAFT" == "false" ]]; then
      echo "Release $NEW_TAG is already published, skipping tag push."
      NEW_TAG=
    fi
  fi
fi
if [[ -n "$NEW_TAG" ]]; then
  # add and push the new tag
  git tag --force $NEW_TAG
  git push --force origin $NEW_TAG
fi
