%define _mingw_bitsize XX (32 or 64)

%if %{_mingw_bitsize} == 32
%define __strip %{_mingw32_strip}
%define __objdump %{_mingw32_objdump}
%define _use_internal_dependency_generator 0
%define __find_requires %{_mingw32_findrequires}
%define __find_provides %{_mingw32_findprovides}
%define __os_install_post %{_mingw32_install_post}
%define _mingw_cmake %{_mingw32_cmake}
%define _mingw_bindir %{_mingw32_bindir}
%else
%define __strip %{_mingw64_strip}
%define __objdump %{_mingw64_objdump}
%define _use_internal_dependency_generator 0
%define __find_requires %{_mingw64_findrequires}
%define __find_provides %{_mingw64_findprovides}
%define __os_install_post %{_mingw64_install_post}
%define _mingw_cmake %{_mingw64_cmake}
%define _mingw_bindir %{_mingw64_bindir}
%endif

Summary:        Virtual Pipe Organ Software
Name:           mingw%{_mingw_bitsize}-grandorgue
BuildRequires:  gcc-c++
BuildRequires:  cmake
BuildRequires:  gettext-tools
BuildRequires:  docbook-xsl-stylesheets 
BuildRequires:  po4a
BuildRequires:  libxslt-tools
BuildRequires:  zip
BuildRequires:  mingw%{_mingw_bitsize}-filesystem >= 23
BuildRequires:  mingw%{_mingw_bitsize}-cross-gcc
BuildRequires:  mingw%{_mingw_bitsize}-cross-gcc-c++
BuildRequires:  mingw%{_mingw_bitsize}-cross-binutils
BuildRequires:  mingw32-cross-nsis
BuildRequires:  mingw%{_mingw_bitsize}-wxwidgets-devel
URL:            http://sourceforge.net/projects/ourorgan/
License:        GPL-2.0+
Group:          Productivity/Multimedia/Sound/Midi
Autoreqprov:    on
Version:        0.3.1.0
Release:        1
Epoch:          0
Source:         grandorgue-%{version}.tar.gz
BuildArch:      noarch
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Recommends:     mingw%{_mingw_bitsize}-grandorgue-demo

%description
 GrandOrgue is a virtual pipe organ sample player application supporting
 a HW1 compatible file format.

 This package contains the mingw%{_mingw_bitsize} build.

%package demo
Summary:        GrandOrgue demo sampleset
Group:          Productivity/Multimedia/Sound/Midi

%description demo
This package contains the demo sampleset for GrandOrgue

%package installer
Summary:        GrandOrgue installer
Group:          Productivity/Multimedia/Sound/Midi

%description installer
 GrandOrgue is a virtual pipe organ sample player application supporting
 a HW1 compatible file format.

 This package contains the mingw%{_mingw_bitsize} installer.

%prep
%setup -q -n grandorgue-%{version}

%build
%{__mkdir} build-tools
cd build-tools
cmake ../src/build
make
cd ..
%define gooptions -DUNICODE=1 -DRTAUDIO_USE_ASIO=OFF  -DIMPORT_EXECUTABLES=../build-tools/ImportExecutables.cmake -DVERSION_REVISION="`echo %{version}|cut -d. -f4`" -DMSYS=1 -DSTATIC=1
%define goinstall GrandOrgue-%{version}-win%{_mingw_bitsize}.exe
%{__mkdir} build-mingw
cd build-mingw
%{_mingw_cmake} .. -DINSTALL_DEPEND=OFF %{gooptions}
make %{?_smp_mflags} VERBOSE=1
cd ..
mkdir build-installer
cd build-installer
%{__mkdir} bin
%{__cp} ../build-mingw/bin/*.exe bin
%{_mingw_cmake} .. -DINSTALL_DEPEND=ON  %{gooptions}
%{__rm} bin/*.exe
make %{?_smp_mflags} VERBOSE=1 package
cd ..

%install
rm -rf %{buildroot}
cd build-mingw
make install DESTDIR=%{buildroot}
cd ..
mkdir -p %{buildroot}
%{__cp} build-installer/GrandOrgue-%{version}-win32.exe %{buildroot}/%{_mingw_bindir}/%{goinstall}
if [ -d %{_topdir}/OTHER ]; then %{__cp} %{buildroot}/%{_mingw_bindir}/%{goinstall} %{_topdir}/OTHER ; fi

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_mingw_bindir}/GrandOrgue.exe
%{_mingw_bindir}/[a-z][a-z]/
%{_mingw_bindir}/help

%files demo
%defattr(-,root,root)
%{_mingw_bindir}/demo

%files installer
%defattr(-,root,root)
%{_mingw_bindir}/%{goinstall}

%changelog
* Tue Nov 15 2011 - martin.koegler@chello.at
- creation
