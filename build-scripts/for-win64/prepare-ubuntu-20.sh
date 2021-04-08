#!/bin/sh

set -e
sudo apt update
sudo DEBIAN_FRONTEND=noninteractive apt install -y zypper cmake g++ pkg-config g++-mingw-w64-x86-64 nsis

cat >/tmp/repo-oss.repo <<EOF
[repo-oss]
name=openSUSE-Tumbleweed-Oss
enabled=1
autorefresh=1
baseurl=http://download.opensuse.org/tumbleweed/repo/oss/  
EOF

cat >/tmp/repo-non-oss.repo  <<EOF
[repo-non-oss]
name=openSUSE-Tumbleweed-Non-Oss
enabled=1
autorefresh=1
baseurl=http://download.opensuse.org/tumbleweed/repo/non-oss/
EOF

cat >/tmp/repo-update.repo <<EOF
[repo-update]
name=openSUSE-Tumbleweed-Update
enabled=1
autorefresh=1
baseurl=http://download.opensuse.org/update/tumbleweed/
EOF

sudo mv /tmp/repo*.repo /etc/zypp/repos.d/

sudo zypper addrepo http://download.opensuse.org/repositories/windows:mingw:win64/openSUSE_Tumbleweed/windows:mingw:win64.repo
sudo zypper addrepo http://download.opensuse.org/repositories/home:e9925248:mingw/openSUSE_Tumbleweed/home:e9925248:mingw.repo
sudo zypper modifyrepo -p 10 home_e9925248_mingw
sudo zypper --gpg-auto-import-keys refresh

sudo mkdir /zypper

sudo zypper --installroot /zypper install -y mingw64-fftw3 mingw64-fftw3-devel mingw64-filesystem mingw64-jack mingw64-jack-devel mingw64-libgnurx-devel mingw64-libgnurx0 mingw64-wavpack mingw64-wavpack-devel mingw64-wxWidgets-3_0-devel mingw64-wxWidgets-3_0-lang mingw64-zlib-devel mingw64-zlib1

sudo ln -s /zypper/usr/x86_64-w64-mingw32/sys-root /usr/x86_64-w64-mingw32/
sudo mv /usr/x86_64-w64-mingw32/sys-root/mingw/lib/libmingw32.a /usr/x86_64-w64-mingw32/sys-root/mingw/lib/libmingw32.save.a
