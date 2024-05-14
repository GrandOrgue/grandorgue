# Installation of GrandOrgue
This document describes installing GrandOrgue from already built installation files. If you want to build these files from sources see [BUILD.md](BUILD.md).

Generally, for installing GrandOrgue or upgrading from a previous version you have to do two steps:

1. Download an installation file from Github.
    * For a release version go to [https://github.com/GrandOrgue/grandorgue/releases](https://github.com/GrandOrgue/grandorgue/releases) and select a release number.
    * For an intermediate build go to [https://github.com/GrandOrgue/grandorgue/actions](https://github.com/GrandOrgue/grandorgue/actions), select a build and scroll down to `Artifacts`
    The necessary file names to download depend on the target operation system.
2. Install the downloaded file or upgrade existing installation with this file. This step is operation system dependent.

These two steps will be described later in the text.

## Warning

Installation of the program creates neither the configuration file nor directories used by GrandOrgue to store data and cache. These directories reside in the user home directory and are created when GrandOrgue is launched the first time.

## Installation or upgrading on Linux

x11 is recommended for running GrandOrgue. Some positioning/sizing may work not
properly on wayland. See more [here](https://github.com/GrandOrgue/grandorgue/issues/1271).

### Run GrandOrgue on Linux directly from AppImage
The GrandOrgue appimage is built for x86_64 architecture without jack support to maximize compatibility. This method does not require any installation of dependencies or GrandOrgue itself.

1. Download the file ``grandorgue-<version>.x86_64.AppImage``
2. Make the appimage file executable (Right click file, select properties, on Permissions tab make sure that Allow executing file as program is ticked. Or open terminal where the file is and run: chmod +x grandorgue*.AppImage)
3. The file is now runnable and can be started by clicking on it

### Installation or upgrading on rpm-based systems (centos, fedora, redhat, oracle linux, opensuse)

1. Download the linux package file ``grandorgue-<version>.<arch>.rpm`` where `<arch>` is

    - `x86_64` for intel 64-bit OS
    - `armhf` for arm 32-bit OS
    - `aarch64` for arm 64-bit OS

2. Run install/upgrade command as a root

    - on opensuse

        - for installation

            ```
            zypper install <path>/grandorgue-<version>.<arch>.rpm
            ```

        - for upgrading

            ```
            zypper update <path>/grandorgue-<version>.<arch>.rpm
            ```

    - on other systems (the same command for installation and upgrading)

        ```
        dnf install <path>/grandorgue-<version>.<arch>.rpm
        ```

    - on old systems

        - for installation

            ```
            yum install <path>/grandorgue-<version>.<arch>.rpm
            ```

        - for upgrading

            ```
            yum update <path>/grandorgue-<version>.<arch>.rpm
            ```

3. After installation, run GrandOrgue from the Applications menu or from the overview screen.

### Installation or upgrading on debian-based systems (debian 9+, ubuntu 18+, mint, raspbian)

1. Download the linux package ``grandorgue_<version>_<arch>.deb`` where `<arch>` is
    - `amd64` for intel 64-bit OS
    - `armhf` for arm 32-bit OS
    - `arm64` for arm 64-bit OS
2. (Debian 9 only): install wx-gtk3

    ```
    echo "deb http://ftp.debian.org/debian stretch-backports main" | sudo tee /etc/apt/sources.list.d/backports.list
    sudo apt-get update
    apt-get install -y -t stretch-backports libwxbase3.0-0v5
    ```

3. Install or upgrade GrandOrgue with all dependencies. Execute the install command as a root:

    ```
    apt-get install <path>/grandorgue_<version>_<arch>.deb
    ```

4. After installation or upgrading, run GrandOrgue from the Applications menu or from the overview screen.

### Running GrandOrgue on Linux without installation
This method requires that all dependencies have already been installed. Usually this way is used for trying a new GrandOrgue version when an old one has already been installed.

1. Download the file ``grandorgue-<version>.linux.<arch>.tar.gz`` where `<arch>` is
    - `amd64` for intel 64-bit OS
    - `armhf` for arm 32-bit OS
    - `aarch64` for arm 64-bit OS
2. Unpack this archive to some directory
3. Run ``bin/GrandOrgue`` from this directory

## Installation or upgrading on Windows 64-bit
1. Download the file ``grandorgue-<version>.windows.x86_64.exe``
2. Run this exe file. The installer will be launched.
3. Follow the prompts.
4. After installation/upgrading, run GrandOrgue from the `Start` menu.

### Running Grandorgue on Windows without installation
1. Download the file ``grandorgue-<version>.windows.x86_64.zip``
2. Unpack it's contents to some directory.
3. Run ``GrandOrgue.exe`` from the `bin` subdirectory

## Installation on macOS
1. Download the file grandorgue-<version>.os.macOS.x86_64.dmg (Intel) or grandorgue-<version>.os.macOS.arm64.dmg (Apple silicon)
2. Double-click the downloaded *.dmg file.
3. Drag and drop the GrandOrgue app to the Applications Folder.
4. GrandOrgue is an app from an unidentified developer. Control-click the GrandOrgue app icon in the Applications Folder, then choose Open from the shortcut menu.
5. Click Open. The GrandOrgue app is saved as an exception to your security settings, and you can open it in the future by double-clicking it just as you can any registered app.

# Deinstallation

## Deinstallation on Linux

Run the following command by root:

- On opensuse:

    ```
    zypper remove grandorgue
    ```

- On new rpm-based systems (centos, fedora, redhat, oracle linux)

    ```
    dnf remove grandorgue
    ```

- On old rpm-based systems

    ```
    yum erase grandorgue
    ```

- On debian-based systems (debian, ubuntu, mint, raspbian)

    ```
    apt-get remove grandorgue
    ```

## Deinstallation on Windows

1. Run ``Control Pannel`` -> `Programs` -> ``Uninstall a progam``.
2. Select ``GrandOrgue``.
3. Follow the prompts.

## Warning

Deinstallation of GrandOrgue removes neither the configuration file nor the directories with GrandOrgue data from user directories.

