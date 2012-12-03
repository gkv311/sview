Name:           sview
Version:        unknown_version
Release:        unknown_release
Summary:        stereoscopic media player sView

License:        GPLv3
URL:            http://www.sview.ru/
Source0:        sview-unknown_version-unknown_release.tar.gz

BuildRequires:  ffmpeg-devel
BuildRequires:  libconfig-devel
BuildRequires:  glew-devel
BuildRequires:  openal-soft-devel
BuildRequires:  gtk+-devel
BuildRequires:  xpm-devel

%description
sView is a stereoscopic Image Viewer and Movie Player.
Requires OpenGL2.0+ for rendering and OpenAL for sound output.

%prep
%setup -q -n %{name}-%{version}-%{release}

%build
make %{?_smp_mflags} INC='-I3rdparty/include -Iinclude -I/usr/include/ffmpeg' all

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT USR_LIB=%{_lib} install

%files
%defattr(-,root,root)
/usr/bin/sView
/usr/%{_lib}/sView/*
/usr/share/application-registry/sView.applications
/usr/share/applications/sViewIV.desktop
/usr/share/applications/sViewMP.desktop
/usr/share/menu/sViewIV
/usr/share/menu/sViewMP
/usr/share/sView/*

%clean
make clean
rm -rf $RPM_BUILD_ROOT

%post
ldconfig

%changelog
* Sat May 19 2012 Kirill Gavrilov
- Initial build
