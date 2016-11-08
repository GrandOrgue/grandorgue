# GrandOrgue - free pipe organ simulator
# 
# Copyright 2006 Milan Digital Audio LLC
# Copyright 2009-2016 GrandOrgue contributors (see AUTHORS)
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

# norootforbuild

Summary:        Virtual Pipe Organ Software
Name:           grandorgue
BuildRequires:  pkgconfig(alsa)
BuildRequires:  gcc-c++
BuildRequires:  jack-audio-connection-kit-devel
BuildRequires:  cmake
%if 0%{?suse_version} >= 1110 
BuildRequires:  wxGTK3-devel
BuildRequires:  gettext-tools
BuildRequires:  docbook-xsl-stylesheets 
BuildRequires:  update-desktop-files
%else
BuildRequires:  wxGTK3-devel
BuildRequires:  gettext
BuildRequires:  docbook-style-xsl
%endif
BuildRequires:  pkgconfig(fftw3f)
BuildRequires:  pkgconfig(libudev)
BuildRequires:  pkgconfig(wavpack)
BuildRequires:  libxslt
BuildRequires:  zip
BuildRequires:  po4a
URL:            http://sourceforge.net/projects/ourorgan/
License:        GPL-2.0+
Group:          Productivity/Multimedia/Sound/Midi
Autoreqprov:    on
Version:        0.3.1.0
Release:        1
Epoch:          0
Source:         grandorgue-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Requires:       grandorgue-resources
%if 0%?suse_version >= 1010
Recommends:     grandorgue-demo
%else
Requires:       grandorgue-demo
%endif

Requires(post):    shared-mime-info desktop-file-utils
Requires(postun):  shared-mime-info desktop-file-utils

%package demo
Summary:        GrandOrgue demo sampleset
License:        GPL-2.0+
Group:          Productivity/Multimedia/Sound/Midi
BuildArch:      noarch

%description
 GrandOrgue is a virtual pipe organ sample player application supporting
 a HW1 compatible file format.

%description demo
This package contains the demo sampleset for GrandOrgue

%package resources
Summary:        GrandOrgue resource files
Group:          Productivity/Multimedia/Sound/Midi
BuildArch:      noarch

%description resources
This package contains the various resource files for GrandOrgue.

%prep
%setup -q

%build
PATH=%{_libexecdir}/wxGTK3:$PATH cmake \
      -DCMAKE_INSTALL_PREFIX=%{_prefix} \
      -DDOC_INSTALL_DIR=%{_docdir} \
      -DLIBINSTDIR=%{_lib} \
      -DCMAKE_SKIP_RPATH=ON \
      -DVERSION_REVISION="`echo %{version}|cut -d. -f4`"
make %{?_smp_mflags} VERBOSE=1

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot} LIBDIR=%{_lib}
mkdir -p %{buildroot}%{_docdir}/%{name}
install -m 644 README* %{buildroot}%{_docdir}/%{name} 
%find_lang GrandOrgue
%if 0%{?suse_version} >= 1110 
%suse_update_desktop_file GrandOrgue
%endif

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}/*
%{_libdir}/libGrandOrgue*
%doc %{_docdir}/%{name}
%{_datadir}/applications/*
%dir %{_datadir}/appdata
%{_datadir}/appdata/*
%{_datadir}/pixmaps/*
%{_datadir}/mime/packages/*
%{_mandir}/man1/*

%files demo
%defattr(-,root,root)
%{_datadir}/GrandOrgue/packages/*.orgue

%files resources -f GrandOrgue.lang
%defattr(-,root,root)
%{_datadir}/GrandOrgue/help
%{_datadir}/GrandOrgue/sounds
%dir %{_datadir}/GrandOrgue/packages
%dir %{_datadir}/GrandOrgue

%post
%if 0%{?suse_version} >= 1140 
%mime_database_post
%desktop_database_post
%else
/usr/bin/update-mime-database %{_datadir}/mime &> /dev/null || :
/usr/bin/update-desktop-database &> /dev/null || :
%endif

%postun
%if 0%{?suse_version} >= 1140 
%mime_database_postun
%desktop_database_postun
%else
/usr/bin/update-mime-database %{_datadir}/mime &> /dev/null || :
/usr/bin/update-desktop-database &> /dev/null || :
%endif

%changelog
* Tue Nov 15 2011 - martin.koegler@chello.at
- creation
