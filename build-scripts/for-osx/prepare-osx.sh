#!/bin/bash

set -e
brew install gettext jack docbook-xsl wxmac cmake pkg-config fftw wavpack imagemagick
brew link gettext --force

# A workaround of https://gitlab.kitware.com/cmake/cmake/-/issues/23826
for F in $(grep -l '(, weak)\?'  /usr/local/Cellar/cmake/*/share/cmake/Modules/GetPrerequisites.cmake); do
  echo Patching $F...
  # OSx sed does not support the -i option
  sed -e 's/(, weak)\?/(, (weak|reexport))?/' $F >${F}.new
  mv ${F}.new $F
done
