[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)

GrandOrgue is a sample based pipe organ simulator.

It currently supports Windows and Linux. Porting to other OS supported by RtMidi, 
RtAudio and wxWidgets should be possible with some build system adjustments.

A) Building on Linux:
--------------------

1) Install gcc C++ compiler, make, cmake and the development packages of wxWigets, jack (libjack), pkg-config, fftw (fftw3), wavpack and alsa (libasound) from your distribution
2) Install docbook-xsl, xsltproc, zip, gettext and po4a (if present on your distribution)
3) Extract the GO sources somewhere, eg: /home/user/gosources
4) Create an empty build directory, eg: mkdir /home/user/gobuild
5) Run cmake:
  cd /home/user/gobuild
  cmake -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" <path to go-sources>
  make

  cmake -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" <path to go-sources>

Hint: For debugging a build, add the -DCMAKE_CXX_FLAGS=-g -DCMAKE_C_FLAGS=-g option to cmake.

Building Linux packages:
-----------------------
To build a deb package (Debian/Ubuntu), do
sudo apt-get install build-essential fakeroot
dpkg-buildpackage -rfakeroot

If dpkg-buildpackage reports, that a package is missing, please install them too (sudo apt-get install <packagename).

GrandOrgue ships a spec file for building RPMs (tested with OpenSuSE and Fedora).
To build, copy the grandorgue tarball to the rpm SOURCE directory and run:
rpmbuild -ba grandorgue.spec

B) Building on OS X
===================

1. Prequisites:
 - OS X >= 10.11
 - Xcode >= 7.3
 - homebrew

2. Install via homebrew:
  - brew update
  - brew install gettext jack docbook-xsl wxmac cmake pkg-config fftw wavpack
  - brew link gettext --force

3. extract the GO source into a directory

4. run the following commands:
  cmake -G "Unix Makefiles" -DCMAKE_CXX_FLAGS=-g -DCMAKE_C_FLAGS=-g -DDOCBOOK_DIR=/usr/local/opt/docbook-xsl/docbook-xsl
  make -k package VERBOSE=1

C) Building for Windows
----------------------

The easiest way to build a Windows version is to cross-compile using a openSuSE 42.1 system (or virtual machine).

a) Cross compiling the existing source rpms

Install OpenSuSE penSuSE Tumbleweed (https://software.opensuse.org/distributions/tumbleweed) (possibly in a Virtual Machine, eg Virtual Box. I would recommend >= 1 GB RAM and >= 20 G of harddisc space).
Run in a terminal:

sudo zypper addrepo http://download.opensuse.org/repositories/windows:mingw:win32/openSUSE_Tumbleweed/windows:mingw:win32.repo
sudo zypper addrepo http://download.opensuse.org/repositories/windows:mingw:win64/openSUSE_Tumbleweed/windows:mingw:win64.repo
sudo zypper addrepo http://download.opensuse.org/repositories/home:e9925248:mingw/openSUSE_Tumbleweed/home:e9925248:mingw.repo
sudo zypper modifyrepo -p 10 home_e9925248_mingw 
sudo zypper refresh
sudo zypper install rpm-build gcc-c++ cmake gettext-tools docbook-xsl-stylesheets po4a libxslt-tools zip

Download the src.rpm from:
http://software.opensuse.org/download.html?project=home%3Ae9925248%3Amingw&package=mingw32-grandorgue
http://software.opensuse.org/download.html?project=home%3Ae9925248%3Amingw&package=mingw64-grandorgue

Install it with

rpm -i mingw??-grandorgue????.src.rpm

If you want to build with asio, change in your home directory in the folder rpmbuild/SPECS in the file mingw??-grandorgue.spec

%define with_asio 0

to

%define with_asio 1

Extract the individual ASIO SDK files to rpmbuild/SOURCES.

Run

rpmbuild -ba rpmbuild/SPECS/mingw??-grandorgue.spec

The first build will complain, that a package is missing. To install them, just run

sudo zypper install <package-name>

Then rerun rpmbuild.

D) Building on Windows
----------------------

The following build procedures works on a 32 or 64 bit windows build system. On a real 64 bit system, it is possible to build the 64 bit version directly without reusing parts from the 32 bit build. In that case -DIMPORT_EXECUTABLES and -DCMAKE_SYSTEM_NAME shall be removed.

All the instructions below are for building GrandOrgue for release, in a single static exe file and using unicode.

As a prerequisite, you need to install CMAKE (http://www.cmake.org) and NSIS (http://nsis.sourceforge.net/Download)

#1: Downloading GrandOrgue sources
	Create a repository with the GrandOrgue sources (SVN : https://ourorgan.svn.sourceforge.net/svnroot/ourorgan)
	In the following example the repository is created at c:\go 

#2: TDM64 environment
	Install TDM64 from http://tdm-gcc.tdragon.net/download
	Install the sources of wxWidgets-3.0.X [use the lastest release] from http://www.wxwidgets.org/downloads/ in C:\TDM\wx32\wxWidgets-3.0.X and C:\TDM\wx64\wxWidgets-3.0.X (same sources at both places)
	Install all the tools for translation and help generation (see Building the help and translations on Windows in README C) ). It is a good idea to copy all exe in the same directory and to add this directory to the Windows path (e.g. C:\TDM\tools\bin, C:\TDM\docbook-xsl-1.76.1 )
	
	Building for 32 bits
		Install msysgit from http://code.google.com/p/msysgit/downloads/detail?name=Git-1.7.8-preview20111206.exe&can=2&q=
		apply wxWidgets-fix-build-3.0.2.patch to c:\TDM\wx32\wxWidgets-3.0.X (right clic on wxWidgets-3.0.X directory to open git bash,  then type git apply /c/go/wxWidgets/wxWidgets-fix-build-3.0.2.patch)
		Wxwidgets:
			change directory to C:\TDM\wx32\wxWidgets-3.0.X\build\msw
			mingw32-make -f makefile.gcc UNICODE=1 BUILD=Release SHARED=0 CXXFLAGS="-m32" CFLAGS="-m32"
		GO:
			create a build directory (e.g. c:\go\build32)
			ASIO support: Either add -DRTAUDIO_USE_ASIO=OFF to the cmake parameters below (no ASIO support), or copy the sources from ASIO SDK at http://www.steinberg.net/en/company/developer.html into C:\go\ext\rt\asio\include (the SDK is free but you will need to set up a developer account with Steinberg).
			you need to replace %1 by GO revision number at the end of the following line, or copy this line in a bat file and use the revision number as the parameter when executing the bat file.
			cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DSTATIC=1 -DwxWidgets_ROOT_DIR=C:\TDM\wx32\wxWidgets-3.0.X -DCMAKE_EXE_LINKER_FLAGS="-m32 -static-libgcc" -DCMAKE_CXX_FLAGS="-m32" -DCMAKE_C_FLAGS="-m32" -DCMAKE_RC_FLAGS="-F pe-i386 -U WIN64" -DDOCBOOK_DIR=C:\TDM\docbook-xsl-1.76.1  -DRTAUDIO_USE_WASAPI=ON -DRTAUDIO_USE_JACK=OFF -DRTMIDI_USE_JACK=OFF -DINSTALL_DEMO=ON -DVERSION_REVISION=%1
			mingw32-make 
			at this point the binary is generated.
			to create the installation package, you need to execute:
			mingw32-make package
			
	Building in 64 bits
		Install msysgit from http://code.google.com/p/msysgit/downloads/detail?name=Git-1.7.8-preview20111206.exe&can=2&q=
		apply wxWidgets-fix-build-3.0.2.patch to c:\TDM\wx64\wxWidgets-3.0.X (right clic on wxWidgets-3.0.X directory to open git bash,  then type git apply /c/go/wxWidgets/wxWidgets-fix-build-3.0.2.patch)
		Wxwidgets:
			change directory to C:\TDM\wx64\wxWidgets-3.0.X\build\msw
			mingw32-make -f makefile.gcc BUILD=Release UNICODE=1 SHARED=0 CXXFLAGS=-DHAVE_VARIADIC_MACROS=1
		GO:
			create a build directory (e.g. c:\go\build64)
			ASIO support: Either add -DRTAUDIO_USE_ASIO=OFF to the cmake parameters below (no ASIO support), or copy the sources from ASIO SDK at http://www.steinberg.net/en/company/developer.html into C:\go\ext\rt\asio\include (the SDK is free but you will need to set up a developer account with Steinberg).
			you need to have build the 32 bit version first since the executables are required to compile the 64 bit version
			you need to replace %1 by GO revision number at the end of the following line, or copy this line in a bat file and use the revision number as the parameter when executing the bat file.
			cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DSTATIC=1 -DwxWidgets_ROOT_DIR=C:\TDM\wx64\wxWidgets-3.0.X -DCMAKE_EXE_LINKER_FLAGS="-static-libgcc" -DIMPORT_EXECUTABLES=c:\go\build32\ImportExecutables.cmake -DCMAKE_SYSTEM_NAME=Windows -DDOCBOOK_DIR=C:\TDM\docbook-xsl-1.76.1 -DRTAUDIO_USE_WASAPI=ON -DINSTALL_DEMO=ON -DRTAUDIO_USE_JACK=OFF -DRTMIDI_USE_JACK=OFF -DVERSION_REVISION=%1 
			mingw32-make 
			at this point the binary is generated.
			to create the installation package, you need to execute:
			mingw32-make package

#3: Cygwin environment: (not maintained)
	Install Cygwin from http://cygwin.com/setup.exe
	You need to select the following packages which are probably not in the basic configuration:
	git (core files), mingw64 (from dev everything except Ada and fortran), make (in Devel GNU utility), cmake (in Devel cross platform build manager), libgtk2-devel (in X11 runtime), docbook-xsl (in text stylesheets), xsltproc (in text libxslt), zip (in Utils utilities), gettext (in Devel library and core utilisties) and po4a. po4a can be obtained from http://sourceware.org/cygwinports/ which provides ports to Cygwin
	Install wxWidgets-3.0.X [use the lastest release] from http://www.wxwidgets.org/downloads/  in c:\cyg\wxsrc32 and c:\cyg\wxsrc64
	create the following directories c/cyg/inst32, inst64, buildlinux32, buildwin32, buildlinux64
	
	Building in 32 bits
		Create toolchain.def in inst32 with following content
			------------------------------------------------------
			# the name of the target operating system
			SET(CMAKE_SYSTEM_NAME Windows)
			SET(MSYS 1)

			# which compilers to use for C and C++
			SET(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
			SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
			SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)

			# here is the target environment located
			SET(CMAKE_FIND_ROOT_PATH  /usr/i686-w64-mingw32 /cygdrive/c/cyg/inst32)

			# adjust the default behaviour of the FIND_XXX() commands:
			# search headers and libraries in the target environment, search 
			# programs in the host environment
			set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
			set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
			set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
			-------------------------------------------------------
		
		Wxwidgets:
			cd /cygdrive/c/cyg/wxsrc32
			apply wxWidgets-fix-build-3.0.2.patch using git apply /cygdrive/c/gosrc/trunk/wxWidgets/wxWidgets-fix-build-3.0.2.patch
			./configure --host=i686-w64-mingw32 --prefix=/cygdrive/c/cyg/inst32 --enable-unicode --disable-shared
			(cd locale && make allmo)
			make
			make install
		GO:
			Create enough of the "native linux" build to get the image converter and its import files created.
				cd /cygdrive/C/cyg/buildlinux32
				cmake -DCMAKE_BUILD_TYPE=Release -DSTATIC=1 -G "Unix Makefiles" /cygdrive/c/gosrc -DCMAKE_INSTALL_PREFIX=/cygdrive/c/cyg/inst32 -DCMAKE_LEGACY_CYGWIN_WIN32=1 -DCMAKE_CXX_FLAGS=-I/usr/include/w32api -DRTAUDIO_USE_WASAPI=OFF -DRTAUDIO_USE_WDMKS=OFF -DRTAUDIO_USE_ASIO=OFF -DRTAUDIO_USE_DSOUND=OFF -DRTAUDIO_USE_WMME=OFF
				make
			Create the 32 bit version of GO
				cd /cygdrive/C/cyg/buildwin32
				cmake -DCMAKE_TOOLCHAIN_FILE=/cygdrive/c/cyg/inst32/toolchain.def /cygdrive/c/gosrc -DCMAKE_BUILD_TYPE=Release -DSTATIC=1 -DCMAKE_INSTALL_PREFIX=/cygdrive/c/cyg/inst32 -DIMPORT_EXECUTABLES=/cygdrive/c/cyg/buildlinux32/ImportExecutables.cmake -DCMAKE_EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++" -DRTAUDIO_USE_WASAPI=OFF 
				make

	Buiding in 64 bits
		Create toolchain.def in inst64 with following content
			------------------------------------------------------
			# the name of the target operating system
			SET(CMAKE_SYSTEM_NAME Windows)
			SET(MSYS 1)

			# which compilers to use for C and C++
			SET(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
			SET(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
			SET(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

			# here is the target environment located
			SET(CMAKE_FIND_ROOT_PATH  /usr/x86_64-w64-mingw32 /cygdrive/c/cyg/inst64)

			# adjust the default behaviour of the FIND_XXX() commands:
			# search headers and libraries in the target environment, search 
			# programs in the host environment
			set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
			set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
			set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)	
			------------------------------------------------------

		Wxwidgets:
			cd /cygdrive/c/cyg/wxsrc64
			apply wxWidgets-fix-build-3.0.2.patch using git apply /cygdrive/c/gosrc/trunk/wxWidgets/wxWidgets-fix-build-3.0.2.patch
			./configure CFLAGS=-m64 CPPFLAGS=-m64 LDFLAGS="-m64 --define WX_CPU_AMD64" --host=x86_64-w64-mingw32 --build=x86_64-pc-cygwin --prefix=/cygdrive/c/cyg/inst64 --enable-unicode --disable-shared
			(cd locale && make allmo)
			make
			make install
		GO:
			Create enough of the "native linux" build to get the image converter and its import files created (unless already done in 32 bit build)
				cd /cygdrive/C/cyg/buildlinux32
				cmake -DCMAKE_BUILD_TYPE=Release -DSTATIC=1 -G "Unix Makefiles" /cygdrive/c/goscr -DCMAKE_INSTALL_PREFIX=/cygdrive/c/cyg/inst32 -DCMAKE_LEGACY_CYGWIN_WIN32=1 -DCMAKE_CXX_FLAGS=-I/usr/include/w32api -DRTAUDIO_USE_WASAPI=OFF -DRTAUDIO_USE_WDMKS=OFF -DRTAUDIO_USE_ASIO=OFF -DRTAUDIO_USE_DSOUND=OFF -DRTAUDIO_USE_WMME=OFF
				make	
		        or create only the build tools:
				cd /cygdrive/C/cyg/buildlinux32
				cmake -G "Unix Makefiles" /cygdrive/c/goscr/src/build -DCMAKE_INSTALL_PREFIX=/cygdrive/c/cyg/inst32 -DCMAKE_LEGACY_CYGWIN_WIN32=1
				make	
			Create the 64 bit version of GO			
				cd ./cygdrive/c/cyg/buildwin64
				cmake -DCMAKE_TOOLCHAIN_FILE=/cygdrive/c/cyg/inst64/toolchain.def /cygdrive/c/gosrc -DCMAKE_BUILD_TYPE=Release -DSTATIC=1 -DCMAKE_INSTALL_PREFIX=/cygdrive/c/cyg/inst64 -DIMPORT_EXECUTABLES=/cygdrive/c/cyg/buildlinux32/ImportExecutables.cmake -DCMAKE_EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++" -DRTAUDIO_USE_WASAPI=OFF 
				make



Cross-compiling for Windows:
----------------------------

1) Install mingw-w64 (On Debian/Ubuntu, install the package mingw-w64) and
   all packages needed to build GO under Linux.
2) Prepare 5 directores:
  * extracted wxWidgets sources [use the lastest release] to a directory (/wxsrc)
  * extract GO trunk sources to a directory (/gosrc)
  * create empty install directory (/inst)
  * create empty win build directory (/buildwin)
  * create empty linux build directory (/buildlinux)
In the following, I use the short path from above - normally
you would put them somewhere under $HOME and use something like /home/user/GO/wxsrc.
3) Build wxWidgets
cd /wxsrc
./configure --host=i686-w64-mingw32 --prefix=/inst --enable-unicode
git apply /gosrc/wxWidgets/wxWidgets-fix-build-3.0.2.patch
(cd locale && make allmo)
make
make install 
4) Build GO for linux
a)
cd /buildlinux
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release /gosrc
make
b) or instead of 4 just build the build tools:
cd /buildlinux
cmake -G "Unix Makefiles" /gosrc/src/build
make
5) Create toolchain definition for windows, file /inst/toolchain.def:
---------------------------------------------------------------------
# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)
SET(MSYS 1)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
SET(PKG_CONFIG_EXECUTABLE i686-w64-mingw32-pkg-config)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH  /usr/i686-w64-mingw32 /inst)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
---------------------------------------------------------------------

6) Build GO for windows
cd /buildwin
cmake -DCMAKE_TOOLCHAIN_FILE=/inst/toolchain.def /gosrc -DCMAKE_INSTALL_PREFIX=/inst -DSTATIC=1 -DIMPORT_EXECUTABLES=/buildlinux/ImportExecutables.cmake -DRTAUDIO_USE_ASIO=OFF
make

-DRTAUDIO_USE_ASIO=OFF turns building ASIO off - else you need to put the ASIO SDK into the sources

7) If you have installed NSIS too, run
make package
to create an installer

If you want to build a 64bit version, replace everywhere in this instruction i686-w64-mingw32 with x86_64-w64-mingw32.

C) Building the help and translations on Windows:
-------------------------------------------------

This is how one can build GrandOrgue help and translations on Windows.

One needs docbook-xsl, gettext, libiconv, libxml2, libxslt, zlib, zip, perl and po4a.

I) Downloads
These packages can be downloaded from the following sites:
docbook-xsl: docbook-xsl-1.76.1.tar.bz2
	http://sourceforge.net/projects/docbook/files/docbook-xsl/1.76.1/docbook-xsl-1.76.1.tar.bz2/download

gettext is made of 4 packages: gettext-runtime, gettext-tools, gettext-runtime-dev, gettext-tools-dev.
All are mandatory to get gettext up and running:
gettext-runtime-dev_0.18.1.1-2_win32.zip
	http://ftp.acc.umu.se/pub/gnome/binaries/win32/dependencies/gettext-runtime-dev_0.18.1.1-2_win32.zip
gettext-runtime_0.18.1.1-2_win32.zip
	http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/gettext-runtime_0.18.1.1-2_win32.zip
gettext-tools-dev_0.18.1.1-2_win32.zip
	http://ftp.acc.umu.se/pub/gnome/binaries/win32/dependencies/gettext-tools-dev_0.18.1.1-2_win32.zip
gettext-tools_0.18.1.1-2_win32.zip
	http://ftp.acc.umu.se/pub/gnome/binaries/win32/dependencies/gettext-tools_0.18.1.1-2_win32.zip

iconv: iconv-1.9.2.win32.zip
	ftp://ftp.zlatkovic.com/libxml/iconv-1.9.2.win32.zip

libxml2: libxml2-2.7.8.win32.zip
	ftp://ftp.zlatkovic.com/libxml/libxml2-2.7.8.win32.zip

libxslt: libxslt-1.1.26.win32.zip
	ftp://ftp.zlatkovic.com/libxml/libxslt-1.1.26.win32.zip

zlib: zlib-1.2.5.win32.zip
	ftp://ftp.zlatkovic.com/libxml/zlib-1.2.5.win32.zip

zip: zip-3.0-setup.exe
	http://sourceforge.net/projects/gnuwin32/files/zip/3.0/zip-3.0-setup.exe/download

perl: perl must be newer than 5.8.2
	http://www.activestate.com => 5.14.2
	http://strawberryperl.com  => 5.12.3
	Both work nicely

po4a: po4a-0.41-1
There exists a cygwin build "ready to install"
	ftp://ftp.cygwinports.org/pub/cygwinports/release-2/po4a/po4a-0.41-1.tar.bz2

II) Installations
All packages can be installed in the same folder except docbook-xsl, perl and po4a

II.1) Install gettext, libiconv, libxml2, libxslt, zlib
Create a new folder (e.g. C:\GNUWin32). One recommends to use a folder name WITHOUT embedded whitespace.
Unzip these packages in C:\GNUWin32.
Note:
zlib-1.2.5.win32.zip, libxslt-1.1.26.win32.zip, libxml2-2.7.8.win32.zip and iconv-1.9.2.win32.zip unzip in
their own folder, so one needs to move the contents of each folder to the upper level C:\GNUWin32

II.2) Install zip and Perl
Run the installers.
One recommends to install perl to C:\Perl (no embedded whitespace) and zip in C:\GNUWin32

II.3) Install po4a
Trouble is sought when trying to run perl scripts in Cygwin Perl from the windows command line, so
po4a must be adapted to windows native Perl. This is easy, because po4a is pure perl.
1) unzip po4a-0.41-1.tar.bz2 to a po4a-0.41-1 folder anywhere on the disk
2) create a new po4a folder anywhere on the disk
2.1) in po4a create folders: bin and site\lib
2.2) copy po4a-0.41-1\usr\bin to po4a\bin
2.3) run Perl's utility pl2bat.bat on
	bin/po4a -> po4a.bat
	bin/po4a-gettextize -> po4a-gettextize.bat
	bin/po4a-normalize -> po4a-normalize.bat
	bin/po4a-translate -> po4a-translate.bat
	bin/po4a-updatepo -> po4a-updatepo.bat
2.4) copy
	po4a-0.41-1\usr\lib\perl5\vendor_perl\5.10\i686-cygwin\auto to po4a\site\lib\auto
	po4a-0.41-1\usr\lib\perl5\vendor_perl\5.10\Locale to po4a\site\lib\Locale
2.5) update Perl installation
	copy po4a\bin contents to perl's bin folder
	copy po4a\site contents to perl's site folder
2.6) optional cleanup : remove po4a and po4a-0.14-1

II.4) Install docbook-xsl
Unzip docbook-xsl-1.76.1.tar.bz2 anywhere on the disk e.g. C:\docbook-xsl-1.76.1

III) Building help
Add GNUWin32 to the system PATH
Check that the Perl bin folder is in the system PATH, since it should have been set by the installer.
If not, add the Perl bin folder to the system PATH.

Run cmake to configure the build:
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release|Debug -DwxWidgets_ROOT_DIR=drive:\path\to\wxWidgets -DDOCBOOK_DIR=drive:\path\to\docbook-xsl-1.76.1
See the README file for further details on the Windows build

There should not be any  of these complaints in the output:
-- Could NOT find Gettext (missing:  GETTEXT_MSGMERGE_EXECUTABLE GETTEXT_MSGFMT_EXECUTABLE)
-- gettext not found
-- xgettext not found (package gettext)
-- msgmerge not found (package gettext)
-- msgfmt not found (package gettext)
-- xsltproc not found
-- zip not found
-- po4a-gettext not found (package po4a)
-- po4a-translate not found (package po4a)
-- Not build help - some programs are missing
-- Not building translations - some programs are missing

Build GrandOrgue by running mingw32-make in the build directory.
See README.translate for further details about adding or updating translations

