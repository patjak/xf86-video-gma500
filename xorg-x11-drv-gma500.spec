Name:           xorg-x11-drv-gma500
Version:        0.8.1
Release:        1%{?dist}
Summary:        GMA500 xorg ddx driver
Group:          System/X11
License:        MIT
URL:            https://github.com/patjak/xf86-video-gma500
Source0:        xf86-video-gma500-master.zip
Source1:        https://github.com/patjak/drm-gma500/raw/blitter_hacking/include/uapi/drm/gma_drm.h
BuildRequires:  xorg-x11-util-macros
BuildRequires:  xorg-x11-server-devel
Requires:       xorg-x11-server-common

%description
Xorg driver for gma500 forked from xf86-video-modesetting.

%prep
%setup -q -n xf86-video-gma500-master
mkdir -p src/uapi/drm
cp %{SOURCE1} src/uapi/drm

%build
./autogen.sh --prefix=/usr
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
%make_install
rm %{buildroot}%{_libdir}/xorg/modules/drivers/gma500_drv.la

%files
%doc README COPYING
%{_libdir}/xorg/modules/drivers/gma500_drv.so
%{_mandir}/man4/gma500.4.*


%changelog
* Thu Jul 17 2014 Wei-Lun Chao <bluebat@member.fsf.org> - 0.8.1
- Initial package
