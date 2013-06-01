%define _mingw_bitsize XX (32 or 64)

%if %{_mingw_bitsize} == 32

%define __strip %{_mingw32_strip}
%define __objdump %{_mingw32_objdump}
%define _use_internal_dependency_generator 0
%define __find_requires %{_mingw32_findrequires}
%define __find_provides %{_mingw32_findprovides}
%define __os_install_post %{_mingw32_install_post}
%define _mingw_configure %{_mingw32_configure}
%define _mingw_bindir %{_mingw32_bindir}
%define _mingw_includedir %{_mingw32_includedir}
%define _mingw_libdir %{_mingw32_libdir}
%define _mingw_datadir %{_mingw32_datadir}

%else

%define __strip %{_mingw64_strip}
%define __objdump %{_mingw64_objdump}
%define _use_internal_dependency_generator 0
%define __find_requires %{_mingw64_findrequires}
%define __find_provides %{_mingw64_findprovides}
%define __os_install_post %{_mingw64_install_post}
%define _mingw_configure %{_mingw64_configure}
%define _mingw_bindir %{_mingw64_bindir}
%define _mingw_includedir %{_mingw64_includedir}
%define _mingw_libdir %{_mingw64_libdir}
%define _mingw_datadir %{_mingw64_datadir}

%endif

Name:           mingw%{_mingw_bitsize}-wxwidgets
Version:        2.8.12
Release:        0
Summary:        wxWidgets toolkit

License:        LGPL-2.0+
Group:          Development/Libraries
URL:            http://www.wxwidgets.org/
Source0:        wxWidgets-%{version}.tar.bz2
Patch1:         wxWidgets-2.8.12.patch
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildArch:      noarch
#!BuildIgnore: post-build-checks  

BuildRequires:  gettext-runtime
BuildRequires:  mingw%{_mingw_bitsize}-filesystem >= 23
BuildRequires:  mingw%{_mingw_bitsize}-cross-gcc
BuildRequires:  mingw%{_mingw_bitsize}-cross-gcc-c++
BuildRequires:  mingw%{_mingw_bitsize}-cross-binutils
BuildRequires:  mingw%{_mingw_bitsize}-libpng-devel
BuildRequires:  mingw%{_mingw_bitsize}-libexpat-devel
BuildRequires:  mingw%{_mingw_bitsize}-libjpeg-devel 
BuildRequires:  mingw%{_mingw_bitsize}-libtiff-devel
BuildRequires:  mingw%{_mingw_bitsize}-zlib-devel

%description
wxWidgets is a free C++ library for cross-platform GUI development.
With wxWidgets, you can create applications for different GUIs (GTK+,
Motif, MS Windows, MacOS X, Windows CE, GPE) from the same source code.

This package contains all libraries of wxWidgets for mingw%{_mingw_bitsize}.


%package devel
Summary:        Everything needed for development with wxWidgets
Group:          Development/Libraries

%description devel
wxWidgets is a free C++ library for cross-platform GUI development.
With wxWidgets, you can create applications for different GUIs (GTK+,
Motif, MS Windows, MacOS X, Windows CE, GPE) from the same source code.

This package contains all files needed for developing with wxWidgets for mingw%{_mingw_bitsize}.


%prep
%setup -q -n wxWidgets-%{version}
%patch1 -p2

%build
MINGW64_CFLAGS="-fno-keep-inline-dllexport %{_mingw64_cflags}" \
MINGW64_CXXFLAGS="-fno-keep-inline-dllexport %{_mingw64_cflags}" \
MINGW32_CFLAGS="-fno-keep-inline-dllexport %{_mingw32_cflags}" \
MINGW32_CXXFLAGS="-fno-keep-inline-dllexport %{_mingw32_cflags}" \
%{_mingw_configure} --enable-unicode

cd locale
make allmo
cd ..
make %{?_smp_mflags} || make

%install
rm -rf $RPM_BUILD_ROOT

make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_mingw_datadir}/locale
%{_mingw_libdir}/*.dll

%files devel
%defattr(-,root,root)
%{_mingw_bindir}/wx-config
%{_mingw_bindir}/wxrc*
%{_mingw_includedir}/wx-2.8
%{_mingw_libdir}/*.a
%{_mingw_libdir}/wx
%{_mingw_datadir}/aclocal
%{_mingw_datadir}/bakefile

%changelog
