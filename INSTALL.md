# Installation of GrandOgue
This document describes installing GrandOrgue from already built installation files. If you want to build these files from sources see [BUILD.md](BUILD.md).

In geneneral for installing GrandOrgue you have to do two steps:

1. Download installation files from Github.
    * For a release version go to `https://github.com/oleg68/GrandOrgue/releases` and select a release number.
    * For an intermediate build go to `https://github.com/oleg68/GrandOrgue/actions`, select a build and scroll down to `Artifacts`
    The necessary file names to download depend on the target operation system.
2. Install the downloaded files. This step is operation system dependent.

These two steps will be described later in the text.

## Installation on Linux
### Installation on rpm-based systems (centos, fedora, redhat, oracle linux, opensuse)
1. Download linux packages
    1. ``grandorgue-<version>.<arch>.rpm``
        where `<arch>` is
        - `x86_64` for intel 64-bit OS
        - `armhf` for arm 32-bit OS
        - `arm64` for arm 64-bit OS
    2. ``grandorgue-resources-<version>.noarch.rpm``
    3. ``grandorgue-demo-<version>.noarch.rpm``
2. Run install command as a root
    - on opensuse

        ```
        zypper install <path>/grandorgue-<version>.<arch>.rpm <path>/grandorgue-resources-<version>.noarch.rpm <path>/grandorgue-demo-<version>.noarch.rpm
        ```

    - on other systems

        ```
        dnf install <path>/grandorgue-<version>.<arch>.rpm <path>/grandorgue-resources-<version>.noarch.rpm <path>/grandorgue-demo-<version>.noarch.rpm
        ```

    - on old systems

        ```
        yum install <path>/grandorgue-<version>.<arch>.rpm <path>/grandorgue-resources-<version>.noarch.rpm <path>/grandorgue-demo-<version>.noarch.rpm
        ```

4. After installation, run GrandOrgue from the Aplication menu or from the overview screen.


### Installation on debian-based systems (debian 9+, ubuntu 18+, mint, raspbian)

1. Download linux packages
    1. ``grandorgue_<version>_<arch>.deb``
        where `<arch>` is
        - `amd64` for intel 64-bit OS
        - `armhf` for arm 32-bit OS
        - `aarch64` for arm 64-bit OS
    2. ``grandorgue-resources_<version>_all.deb``
    3. ``grandorgue-demo_<version>_all.deb``
2. (Debian 9 only): install wx-gtk3

    ```
    echo "deb http://ftp.debian.org/debian stretch-backports main" | sudo tee /etc/apt/sources.list.d/backports.list
    sudo apt-get update
    apt-get install -y -t stretch-backports libwxbase3.0-0v5
    ```

3. Install GrandOrgue with all dependencies. Execute the install command as a root:

    ```
    apt-get install <path>/grandorgue_<version>_<arch>.deb <path>/grandorgue-resources_<version>_all.deb <path>/grandorgue-demo_<version>_all.deb
    ```

4. After installation, run GrandOrgue from the Aplication menu or from the overview screen.

### Running GrandOrgue on Linux without installation
This method requires that all dependencies have already been installed. Usually this way is used for trying a new GrandOrgue version when an old one has been installed.

1. Download the file ``grandorgue-<version>.linux.<arch>.tar.gz
    where `<arch>` is
    - `amd64` for intel 64-bit OS
    - `armhf` for arm 32-bit OS
    - `aarch64` for arm 64-bit OS
2. Unpack this archive to some directory
3. Run ``bin/GrandOrgue``
## Installation on Windows 64-bit
1. Download the file ``grandorgue-<version>.windows.x86_64.exe``
2. Run this exe file. The installer will be launched.
3. Follow the prompts.
4. After installation, run GrandOrgue from the `Start` menu.
### Running Grandorgue on Windows without installation
1. Download the file ``grandorgue-<version>.windows.x86_64.zip``
2. Unpack it's contents to some directory.
3. Run ``GrandOrgue.exe`` from the `bin` subdirecttory
## Installation on OSx (intel-based only)
1. Download the file grandorgue-<version>.os.osx.x86_64.dmg
2. Open it
3. Click on the keyboard icon for running