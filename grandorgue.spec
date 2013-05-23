# norootforbuild

Summary:        Virtual Pipe Organ Software
Name:           grandorgue
BuildRequires:  alsa-lib-devel
BuildRequires:  gcc-c++
BuildRequires:  jack-audio-connection-kit-devel
BuildRequires:  cmake
%if 0%{?suse_version} >= 1110 
BuildRequires:  wxWidgets-devel
BuildRequires:  gettext-tools
BuildRequires:  docbook-xsl-stylesheets 
BuildRequires:  update-desktop-files
%else
BuildRequires:  wxGTK-devel
BuildRequires:  gettext
BuildRequires:  docbook-style-xsl
%endif
BuildRequires:  libxslt
BuildRequires:  zip
# po5a not in the main repository (only http://download.opensuse.org/repositories/M17N/). To build without delete the po4a build requirement
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

Requires(post):    shared-mime-info desktop-file-utils
Requires(postun):  shared-mime-info desktop-file-utils

%package demo
Summary:        GrandOrgue demo sampleset
License:        GPL-2.0+
Group:          Productivity/Multimedia/Sound/Midi
Recommends:     grandorgue-demo
BuildArch:      noarch

%description
 GrandOrgue is a virtual pipe organ sample player application supporting
 a HW1 compatible file format.

%description demo
This package contains the demo sampleset for GrandOrgue

%prep
%setup -q

%build
cmake -DUNICODE=1 \
      -DCMAKE_INSTALL_PREFIX=%{_prefix} \
      -DDOC_INSTALL_DIR=%{_docdir} \
      -DLIB=%{_lib} \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_SKIP_RPATH=1 \
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

%files -f GrandOrgue.lang
%defattr(-,root,root)
%{_bindir}/*
%doc %{_docdir}/%{name}
%{_datadir}/GrandOrgue/help
%{_datadir}/applications/*
%{_datadir}/pixmaps/*
%{_datadir}/mime/packages/*
%{_mandir}/man1/*

%files demo
%{_datadir}/GrandOrgue/demo

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
