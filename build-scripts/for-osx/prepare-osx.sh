#!/bin/bash

set -e
brew install \
  cmake \
  docbook-xsl \
  fftw wavpack \
  gettext \
  imagemagick \
  jack \
  pkg-config \
  yaml-cpp
brew link gettext --force

# install wx for 10.15 in order to using the GrandOrgue bundle with 10.15
# prevent upgrading wxwidgets just afrer installing
export HOMEBREW_NO_INSTALLED_DEPENDENTS_CHECK=1
brew install -f --formula https://github.com/GrandOrgue/WxOsX/releases/download/3.2.0-1/90dab952e7e1c26dba572e1b19b7f97728cbbd4d5580a020aee0ff36eebe9138--wxwidgets--3.2.0.catalina.bottle.tar.gz

# A workaround of https://gitlab.kitware.com/cmake/cmake/-/issues/23826
for F in $(grep -l '(, weak)\?'  /usr/local/Cellar/cmake/*/share/cmake/Modules/GetPrerequisites.cmake); do
  echo Patching $F...
  # OSx sed does not support the -i option
  sed -e 's/(, weak)\?/(, (weak|reexport))?/' $F >${F}.new
  mv ${F}.new $F
done
