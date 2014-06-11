# GrandOrgue - free pipe organ simulator
# 
# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2014 GrandOrgue contributors (see AUTHORS)
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

pkgname=grandorgue
pkgver=0.3.1.0
pkgrel=0
pkgdesc=('Virtual Pipe Organ Software')
arch=('i686' 'x86_64')
url='http://sourceforge.net/projects/ourorgan/'
license=('GPL2+')
groups=('multimedia')
depends=('alsa-lib' 'jack' 'wxgtk2.8')
makedepends=('gcc' 'libxslt' 'zip' 'gettext' 'docbook-xsl' 'cmake')
source=("grandorgue-$pkgver.tar.gz")
md5sums=('SKIP')

build()
{
    cd "${srcdir}/grandorgue-${pkgver}"
    mkdir build
    cd build
    cmake -DUNICODE=1 \
	-DCMAKE_INSTALL_PREFIX=/usr \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_SKIP_RPATH=1 \
	-DVERSION_REVISION="`echo "$pkgver"|cut -d. -f4`" \
	-D wxWidgets_CONFIG_EXECUTABLE:string=/usr/bin/wx-config-2.8 \
      ..
    make  VERBOSE=1
}

package()
{
    cd "${srcdir}/grandorgue-${pkgver}"
    cd build
    make install DESTDIR="$pkgdir/"
}
