# Building GrandOrgue from sources
## Obtaining the source code
### With git
The simplest way of getting the source code is to use `git`.

1. Go to some directory where the GrandOgue source subdirectory will be created
2. Clone the main GrandOrgue repository

    ```
    git clone --recurse-submodules https://github.com/GrandOrgue/grandorgue.git
    ```

The source code will be fetched in the GrandOrgue subdirectory of the current directory

### Manually
You can download the source code archive from GitHub

1. Download archive
    - For getting the last sources from `master` branch go to the [Download  ZIP](https://github.com/GrandOrgue/grandorgue/archive/refs/heads/master.zip) on GitHub
    - For getting sourcecode of a certain GrandOrgue release go to the [Releases page](https://github.com/GrandOrgue/grandorgue/releases), select the particular release, scroll down to `Assets` and download the ``Source code (zip)`` or ``Source code (tar.gz)``

2. Unpack this archive to some directory

3. Download external submodules
    1. `RtAudio`: Download [the source archive](https://github.com/thestk/rtaudio/archive/refs/heads/master.zip) end extract the contents of the ``rtaudio-master`` subdirectory from the archive to the ``submodules/RtAudio`` subdirectory of GrandOrgue source tree.
    2. `RtMidi`: Download [the source archive](https://github.com/thestk/rtmidi/archive/refs/heads/master.zip) and extract the contents of the ``rtmidi-master`` subdirectory from the archive to the ``submodules/RtMidi`` subdirectory of GrandOrgue source tree.
    3. `PortAudio`: Download [the source archive](https://github.com/PortAudio/portaudio/archive/refs/heads/master.zip) and extract the contents of the ``portaudio-master`` subdirectory from the archive to the ``submodules/PortAudio`` subdirectory of GrandOrgue source tree.
    4. `ZitaConvolver`: Download [the source archive](https://salsa.debian.org/multimedia-team/zita-convolver/-/archive/master/zita-convolver-master.zip) and extract the contents of the ``zita-convolver-master`` subdirectory from the archive to the ``submodules/ZitaConvolver`` subdirectory of GrandOrgue source tree.

## Building for Linux on Linux
1. Make sure that GrandOrgue source tree has been extracted to some subdirectory ``<GO source tree>``
2. Install required software
    - Manually
        1. Install gcc C++ compiler, make, cmake, imagemagic, pkg-config
        2. Install the development packages of wxWigets, alsa (libasound), jack (libjack), fftw (fftw3), wavpack, yaml-cpp, zlib, libcurl, udev from your distribution.
        
            The exact name of packages differ from one distribution to another.             For example, on any debian-based distribution (including Ubuntu, Mint and Raspbian) they are libasound2-dev, libfftw3-dev, libjack-dev, libudev-dev, libwxgtk3.2-dev, libyaml-cpp-dev, zlib1g-dev, libcurl4-openssl-dev.
        3. Install docbook-xsl, xsltproc, zip, gettext and po4a (if present on your distribution).
    - Or run the prepared scripts for certain linux distributions by a sudoer user:
        - on Fedora run ``<GO source tree>/build-scripts/for-linux/prepare-fedora.sh``
        - on OpenSuse run ``<GO source tree>/build-scripts/for-linux/prepare-opensuse.sh``
        - on Debian 10+ or on Ubuntu 20+ run ``<GO source tree>/build-scripts/for-linux/prepare-debian-based.sh``
3. Build
    - Manually
        1. Create an empty build directory, eg:

            ```
            mkdir $HOME/gobuild
            ```

        2. Run cmake


            ```
            cd /home/user/gobuild
            cmake -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" <GO source tree>
            ```

            Hint: For debugging a build, add the -DCMAKE_CXX_FLAGS=-g -DCMAKE_C_FLAGS=-g option to cmake.


        3. Run make


            ```
            make
            ```

        4. For making tar.gz, rpm and deb packages, run


            ```
            make package
            ```

        5. For making rpm source package, run

            ```
            cpack -G RPM --config ./CPackSourceConfig.cmake
            ```

    - Or run the prepared build script


        ```
        <GO source tree>/build-scripts/for-linux/build-on-linux.sh
        ```

        The built packages will appear in the build/linux subdirectory of current directory, the executables - in build/linux/bin

## Building AppImage for Linux on Linux (x86_64, without jack support)
1. Make sure that GrandOrgue source tree has been extracted to some subdirectory ``<GO source tree>``
2. Install required software
    - Manually
        1. Install gcc C++ compiler, make, cmake, imagemagic and the development packages of wxWigets, pkg-config, fftw (fftw3), wavpack and alsa (libasound) from your distribution
        2. Install docbook-xsl, xsltproc, zip, gettext and po4a (if present on your distribution)
        3. Install patchelf, libgtk-3-dev and librsvg2-dev from your distribution (if you want to use linuxdeploy-plugin-gtk, otherwise you don't need them and can ignore all the plugin related commands below)
        4. Install linuxdeploy, linuxdeploy-plugin-gtk and appimagetool somewhere in path like /usr/local/bin
           (or $HOME/bin in which case the use of sudo below should be ommitted and the patch changed accordingly)
           ```
           sudo wget -P /usr/local/bin https://raw.githubusercontent.com/linuxdeploy/linuxdeploy-plugin-gtk/master/linuxdeploy-plugin-gtk.sh
           sudo wget -P /usr/local/bin https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
           sudo wget -P /usr/local/bin https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage

           sudo chmod +x /usr/local/bin/linuxdeploy-plugin-gtk.sh
           sudo chmod +x /usr/local/bin/linuxdeploy-x86_64.AppImage
           sudo chmod +x /usr/local/bin/appimagetool-x86_64.AppImage
           ```
    - Or run the prepared script by a sudoer user:
        - on Debian 9+ or on Ubuntu 18+ run ``<GO source tree>/build-scripts/for-appimage-x86_64/prepare-debian-ubuntu.sh``
3. Build
    - Manually
        1. Create an empty build directory, eg:

            ```
            mkdir $HOME/gobuild
            ```

        2. Run cmake

            ```
            cd /home/user/gobuild
            cmake -DCMAKE_INSTALL_PREFIX=/usr -DRTAUDIO_USE_JACK=OFF -DRTMIDI_USE_JACK=OFF -DGO_USE_JACK=OFF -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" <GO source tree>
            ```

        3. Run make

            ```
            make
            ```

        4. For installing into AppDir

            ```
            make install DESTDIR=AppDir
            ```

        5. For initializing AppDir and building AppImage

            ```
            linuxdeploy-x86_64.AppImage --appdir AppDir --plugin gtk
            appimagetool-x86_64.AppImage --no-appstream AppDir grandorgue-<version_number>-<build_number>.x86_64.AppImage
            ```

            Note: On a local AppImage build you might get better results by ommitting the linuxdeploy-plugin-gtk completely, and you can ommit specifying the appimage output name like so

            ```
            linuxdeploy-x86_64.AppImage --appdir AppDir
            appimagetool-x86_64.AppImage --no-appstream AppDir
            ```
            
            The appimage will in any way appear in the build directory.

    - Or run the prepared build script

        ```
        <GO source tree>/build-scripts/for-appimage-x86_64/build-on-linux.sh
        ```

        The built appimage will appear in the build/appimage-x86_64 subdirectory of current directory

## Building for macOS on macOS

1. Prequisites:
    1. A macOS version supported by Homebrew (the three latest major versions of macOS)
    2. Xcode or Command Line Tools for Xcode
    3. Homebrew

2. Extract the GO sources somewhere, eg: ~/documents/Projects/GrandOrgueDev/gosources using the Git method. The manual method may install the wrong version of an external submodule.

3. Install the required software by running (the commands in)
    ```
    <GO source tree>/build-scripts/for-osx/prepare-osx.sh
    ```

4. Build
    - Manually
        1. Create an empty build folder, eg: ~/documents/Projects/GrandOrgueDev/gobuild

        2. Change the working directory
            ```
            cd ~/documents/Projects/GrandOrgueDev/gobuild
            ```

        3. Run cmake:
            - on Apple silicon
                ```
                cmake -G "Unix Makefiles" -DDOCBOOK_DIR=/opt/homebrew/opt/docbook-xsl/docbook-xsl <GO source tree>
                ```
            - on Intel
                ```
                cmake -G "Unix Makefiles" -DDOCBOOK_DIR=/usr/local/opt/docbook-xsl/docbook-xsl <GO source tree>
                ```
            Hint: For debugging a build, add the ``-DCMAKE_CXX_FLAGS=-g -DCMAKE_C_FLAGS=-g`` option to `cmake`.

        4. Run make
            ```
            make
            ```

    - Automatically
        1. Create an empty build folder, eg: ~/documents/Projects/GrandOrgueDev/gobuild

        2. Change the working directory
            ```
            cd ~/documents/Projects/GrandOrgueDev/gobuild
            ```

        3. Run the prepared build script
            ```
            <GO source tree>/build-scripts/for-osx/build-on-osx.sh
            ```
            The built app will appear in the build/osx subdirectory of current directory.

## Cross-building for Windows-64 bit on Linux

1. Install mingw-w64 (On Debian/Ubuntu, install the package mingw-w64) and all packages needed to build GO under Linux.
2. Prepare 5 directores:
    1. extracted wxWidgets sources [use the lastest release] to a directory (/wxsrc)
    2. extract GO trunk sources to a directory (/gosrc)
    3. create empty install directory (/inst)
    4. create empty win build directory (/buildwin)
    5. create empty linux build directory (/buildlinux)


    In the following, I use the short path from above - normally you would put them somewhere under $HOME and use something like /home/user/GO/wxsrc.

3. Build wxWidgets

    ```
    cd /wxsrc
    ./configure --host=i686-w64-mingw32 --prefix=/inst --enable-unicode
    git apply /gosrc/wxWidgets/wxWidgets-fix-build-3.0.2.patch
    (cd locale && make allmo)
    make
    make install
    ```

4. Build the tools for linux

    ```
    cd /buildlinux
    cmake -G "Unix Makefiles" /gosrc/src/build
    make
    ```

5. Create toolchain definition for windows, file /inst/toolchain.def:

    ```
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
    ```

6. Build GO for windows

    ```
    cd /buildwin
    cmake -DCMAKE_TOOLCHAIN_FILE=/inst/toolchain.def /gosrc -DCMAKE_INSTALL_PREFIX=/inst -DSTATIC=1 -DIMPORT_EXECUTABLES=/buildlinux/ImportExecutables.cmake -DRTAUDIO_USE_ASIO=OFF
    make
    ```

    -DRTAUDIO_USE_ASIO=OFF turns building ASIO off - else you need to put the ASIO SDK into the sources

7. If you have installed NSIS too, run

    ```
    make package
    ```

    to create an installer

- If you want to build a 64bit version, replace everywhere in this instruction i686-w64-mingw32 with x86_64-w64-mingw32.

### Simplified cross-compiling for Windows on Linux

There are some ready-to use bash scripts that allow to build windows executables on linux.
Assume you have the GrandOrgue source extracted to GrandOrgue directory

1. Install the necessary software running a prepare-script by root or a sudoer-user
    - If you have OpenSuse, run


        ```
        GrandOrgue/build-scripts/for-win64/prepare-opensuse.sh
        ```

	- If you have Ubuntu 20.04+, run `GrandOrgue/build-scripts/for-win64/prepare-ubuntu-20.sh`

2. run build:

    ```
    GrandOrgue/build-scripts/for-win64/build-on-linux.sh
    ```

    Windows executables will appear in the build/win64 subdirectory of the current directory
