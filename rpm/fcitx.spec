%global _enable_debug_package 0
%global debug_package %{nil}
%global __os_install_post /usr/lib/rpm/brp-compress %{nil}
%global _xinputconf %{_sysconfdir}/X11/xinit/xinput.d/fcitx.conf
%global _fcitxsh %{_sysconfdir}/X11/xinit/xinitrc.d/99_fcitx.sh
%{!?gtk2_binary_version: %global gtk2_binary_version %(pkg-config  --variable=gtk_binary_version gtk+-2.0)}
%{!?gtk3_binary_version: %global gtk3_binary_version %(pkg-config  --variable=gtk_binary_version gtk+-3.0)}

Name:			fcitx
Summary:		An input method framework
Version:		4.2.9.31.79
Release:		1%{?dist}
License:		GPLv2+
URL:			https://fcitx-im.org/wiki/Fcitx
Source0:		%{name}-%{version}.tar.gz
#Source1:		xinput_%%{name}
BuildRequires:		gcc-c++
BuildRequires:		pango-devel, dbus-devel
%if 0%{?rhel} < 8
#BuildRequires:		opencc-devel
#BuildRequires:		qt4-devel
#BuildRequires:		enchant-devel
%else
%ifarch aarch64 s390x
%else
BuildRequires:		enchant-devel
%endif
%endif
BuildRequires:		wget, intltool, chrpath
BuildRequires:		cmake, libtool, doxygen, libicu-devel
BuildRequires:		gtk3-devel, gtk2-devel, libicu
BuildRequires:		xorg-x11-proto-devel, xorg-x11-xtrans-devel
BuildRequires:		gobject-introspection-devel, libxkbfile-devel
BuildRequires:		iso-codes-devel, libicu-devel
BuildRequires:		libX11-devel, dbus-glib-devel, dbus-x11
BuildRequires:		desktop-file-utils, libxml2-devel
BuildRequires:		lua-devel, extra-cmake-modules
BuildRequires:		xkeyboard-config-devel
BuildRequires:		libuuid-devel
BuildRequires:		json-c-devel
Requires:		%{name}-data = %{version}-%{release}
Requires:		imsettings
Requires(post):		%{_sbindir}/alternatives
Requires(postun):	%{_sbindir}/alternatives
Requires:		%{name}-libs%{?_isa} = %{version}-%{release}
Requires:		%{name}-gtk3 = %{version}-%{release}
Requires:		%{name}-gtk2 = %{version}-%{release}

%description
Fcitx is an input method framework with extension support. Currently it
supports Linux and Unix systems like FreeBSD.

Fcitx tries to provide a native feeling under all desktop as well as a light
weight core. You can easily customize it to fit your requirements.

%package data
Summary:		Data files of Fcitx
BuildArch:		noarch
Requires:		hicolor-icon-theme
Requires:		dbus

%description data
The %{name}-data package provides shared data for Fcitx.

%package libs
Summary:		Shared libraries for Fcitx
Provides:		%{name}-keyboard = %{version}-%{release}
Obsoletes:		%{name}-keyboard =< 4.2.3

%description libs
The %{name}-libs package provides shared libraries for Fcitx

%package devel
Summary:		Development files for Fcitx
Requires:		%{name}-libs%{?_isa} = %{version}-%{release}
Requires:		libX11-devel%{?_isa}

%description devel
The %{name}-devel package contains libraries and header files necessary for
developing programs using Fcitx libraries.

%package table-chinese
Summary:		Chinese table of Fcitx
BuildArch:		noarch
Requires:		%{name}-table = %{version}-%{release}

%description table-chinese
The %{name}-table-chinese package provides other Chinese table for Fcitx.

%package gtk2
Summary:		Fcitx IM module for gtk2
Requires:		%{name} = %{version}-%{release}
Requires:		%{name}-libs%{?_isa} = %{version}-%{release}

%description gtk2
This package contains Fcitx IM module for gtk2.

%package gtk3
Summary:		Fcitx IM module for gtk3
Requires:		%{name} = %{version}-%{release}
Requires:		%{name}-libs%{?_isa} = %{version}-%{release}
Requires:		imsettings-gnome

%description gtk3
This package contains Fcitx IM module for gtk3.

%if 0%{?rhel} < 8
#%package qt4
#Summary:		Fcitx IM module for qt4
#Requires:		%%{name} = %%{version}-%%{release}
#Requires:		%%{name}-libs%%{?_isa} = %%{version}-%%{release}

#%description qt4
#This package contains Fcitx IM module for qt4.
%endif

%package pinyin
Summary:		Pinyin Engine for Fcitx
URL:			https://fcitx-im.org/wiki/Built-in_Pinyin
Requires:		%{name} = %{version}-%{release}
Requires:		%{name}-libs%{?_isa} = %{version}-%{release}
Requires:		%{name}-data = %{version}-%{release}

%description pinyin
This package contains pinyin engine for Fcitx.

%package qw
Summary:		Quwei Engine for Fcitx
URL:			https://fcitx-im.org/wiki/QuWei
Requires:		%{name} = %{version}-%{release}
Requires:		%{name}-libs%{?_isa} = %{version}-%{release}
Requires:		%{name}-data = %{version}-%{release}

%description qw
This package contains Quwei engine for Fcitx.

%package table
Summary:		Table Engine for Fcitx
URL:			https://fcitx-im.org/wiki/Table
Requires:		%{name} = %{version}-%{release}
Requires:		%{name}-libs%{?_isa} = %{version}-%{release}
Requires:		%{name}-data = %{version}-%{release}
Requires:		%{name}-pinyin = %{version}-%{release}

%description table
This package contains table engine for Fcitx.

%prep
# cp %{_specdir}/xinput_fcitx %{_sourcedir}

cat > %{_sourcedir}/xinput_fcitx <<EOF
XIM=fcitx
XIM_PROGRAM=/usr/bin/fcitx
ICON="/usr/share/pixmaps/fcitx.png"
XIM_ARGS="-D"
PREFERENCE_PROGRAM=/usr/bin/fcitx-configtool
SHORT_DESC="FCITX"
GTK_IM_MODULE=fcitx
if test -f /usr/lib/qt4/plugins/inputmethods/qtim-fcitx.so || \
   test -f /usr/lib64/qt4/plugins/inputmethods/qtim-fcitx.so || \
   test -f /usr/lib/qt5/plugins/platforminputcontexts/libfcitxplatforminputcontextplugin.so || \
   test -f /usr/lib64/qt5/plugins/platforminputcontexts/libfcitxplatforminputcontextplugin.so;
then
    QT_IM_MODULE=fcitx
else
    QT_IM_MODULE=xim
fi
EOF

# cp %{_specdir}/99_fcitx.sh %{_sourcedir}

cat > %{_sourcedir}/99_fcitx.sh <<EOF
#!/bin/sh

systemctl --user import-environment GTK_IM_MODULE QT_IM_MODULE XMODIFIERS

if command -v dbus-update-activation-environment >/dev/null 2>&1; then
    dbus-update-activation-environment GTK_IM_MODULE QT_IM_MODULE XMODIFIERS
fi
EOF

%setup -q

%build
mkdir -p build
pushd build
cmake .. -DENABLE_GTK3_IM_MODULE=On -DENABLE_QT_IM_MODULE=On -DENABLE_QT=Off -DENABLE_OPENCC=Off -DENABLE_ENCHANT=Off -DENABLE_LUA=On -DENABLE_GIR=On  -DENABLE_XDGAUTOSTART=Off  -DCMAKE_INSTALL_PREFIX:PATH=/usr -DLIB_INSTALL_DIR=/usr/lib64 

make VERBOSE=1 %{?_smp_mflags}

%install
%make_install INSTALL="install -p" -C build

#find %{buildroot}%{_libdir} -name '*.la' -delete -print

install -pm 644 -D %{_sourcedir}/99_fcitx.sh %{buildroot}%{_fcitxsh}
install -pm 644 -D %{_sourcedir}/xinput_fcitx %{buildroot}%{_xinputconf}

# patch fcitx4-config to use pkg-config to solve libdir to avoid multiarch
# confilict
sed -i -e 's:%{_libdir}:`pkg-config --variable=libdir fcitx`:g' \
  %{buildroot}%{_bindir}/fcitx4-config

chmod +x %{buildroot}%{_datadir}/%{name}/data/env_setup.sh

%find_lang %{name}

desktop-file-install --delete-original \
  --dir %{buildroot}%{_datadir}/applications \
  %{buildroot}%{_datadir}/applications/%{name}-skin-installer.desktop

desktop-file-install --delete-original \
  --dir %{buildroot}%{_datadir}/applications \
  %{buildroot}%{_datadir}/applications/%{name}-configtool.desktop

desktop-file-install --delete-original \
  --dir %{buildroot}%{_datadir}/applications \
  %{buildroot}%{_datadir}/applications/%{name}.desktop

%post 
%{_sbindir}/alternatives --install %{_sysconfdir}/X11/xinit/xinputrc xinputrc %{_xinputconf} 55 || :
%{_sbindir}/alternatives --install %{_sysconfdir}/X11/xinit/99_fcitx.sh 99_fcitx.sh %{_fcitxsh} 55 || :

%postun  
if [ "$1" = "0" ]; then
  %{_sbindir}/alternatives --remove xinputrc %{_xinputconf} || :
  # if alternative was set to manual, reset to auto
  [ -L %{_sysconfdir}/alternatives/xinputrc -a "`readlink %{_sysconfdir}/alternatives/xinputrc`" = "%{_xinputconf}" ] && %{_sbindir}/alternatives --auto xinputrc || :
fi
if [ "$1" = "0" ]; then
  %{_sbindir}/alternatives --remove 99_fcitx.sh %{_fcitxsh} || :
  # if alternative was set to manual, reset to auto
  [ -L %{_sysconfdir}/alternatives/99_fcitx.sh -a "`readlink %{_sysconfdir}/alternatives/99_fcitx.sh`" = "%{_fcitxsh}" ] && %{_sbindir}/alternatives --auto 99_fcitx.sh || :
fi

%ldconfig_scriptlets libs

%files -f %{name}.lang
%doc AUTHORS ChangeLog THANKS TODO
%license COPYING
%config %{_xinputconf}
%config %{_fcitxsh}
%{_bindir}/fcitx-*
%{_bindir}/fcitx
%{_bindir}/createPYMB
%{_bindir}/mb2org
%{_bindir}/mb2txt
%{_bindir}/readPYBase
%{_bindir}/readPYMB
%{_bindir}/scel2org
%{_bindir}/txt2mb
%{_datadir}/applications/%{name}-skin-installer.desktop
%dir %{_datadir}/%{name}/dbus/
%{_datadir}/%{name}/dbus/daemon.conf
%{_datadir}/applications/%{name}-configtool.desktop
%{_datadir}/applications/%{name}.desktop
%{_datadir}/mime/packages/x-fskin.xml
%{_mandir}/man1/createPYMB.1*
%{_mandir}/man1/fcitx-remote.1*
%{_mandir}/man1/fcitx*
%{_mandir}/man1/mb2org.1*
%{_mandir}/man1/mb2txt.1*
%{_mandir}/man1/readPYBase.1*
%{_mandir}/man1/readPYMB.1*
%{_mandir}/man1/scel2org.1*
%{_mandir}/man1/txt2mb.1*

%files libs
%license COPYING
%{_libdir}/libfcitx*.so.*
%dir %{_libdir}/%{name}/
%{_libdir}/%{name}/%{name}-[!pqt]*.so
%{_libdir}/%{name}/%{name}-punc.so
%{_libdir}/%{name}/%{name}-quickphrase.so
%{_libdir}/%{name}/libexec/
%dir %{_libdir}/girepository-1.0/
%{_libdir}/girepository-1.0/Fcitx-1.0.typelib

%files data
%license COPYING
%{_datadir}/icons/hicolor/16x16/apps/*.png
%{_datadir}/icons/hicolor/22x22/apps/*.png
%{_datadir}/icons/hicolor/24x24/apps/*.png
%{_datadir}/icons/hicolor/32x32/apps/*.png
%{_datadir}/icons/hicolor/48x48/apps/*.png
%{_datadir}/icons/hicolor/128x128/apps/*.png
%{_datadir}/icons/hicolor/scalable/apps/*.svg
%dir %{_datadir}/%{name}
%{_datadir}/%{name}/skin/
%dir %{_datadir}/%{name}/addon
%{_datadir}/%{name}/addon/%{name}-[!pqt]*.conf
%{_datadir}/%{name}/addon/%{name}-punc.conf
%{_datadir}/%{name}/addon/%{name}-quickphrase.conf
%{_datadir}/%{name}/data/
%{_datadir}/%{name}/spell/
%dir %{_datadir}/%{name}/imicon/
%dir %{_datadir}/%{name}/inputmethod/
%dir %{_datadir}/%{name}/configdesc/
%dir %{_datadir}/%{name}/table/
%{_datadir}/%{name}/configdesc/[!ft]*.desc
%{_datadir}/%{name}/configdesc/fcitx-[!p]*.desc
%{_datadir}/dbus-1/services/org.fcitx.Fcitx.service

%files devel
%license COPYING
%{_bindir}/fcitx4-config
%{_libdir}/libfcitx*.so
%{_libdir}/pkgconfig/fcitx*.pc
%{_includedir}/fcitx*
%{_datadir}/cmake/%{name}/
%{_docdir}/%{name}/*
%dir %{_datadir}/gir-1.0
%{_datadir}/gir-1.0/Fcitx-1.0.gir

%files table-chinese
%license COPYING
%doc
%{_datadir}/%{name}/table/*
%{_datadir}/%{name}/imicon/[!ps]*.png

%files pinyin
%license COPYING
%doc
%{_datadir}/%{name}/inputmethod/pinyin.conf
%{_datadir}/%{name}/inputmethod/shuangpin.conf
%{_datadir}/%{name}/pinyin/
%{_datadir}/%{name}/configdesc/fcitx-pinyin.desc
%{_datadir}/%{name}/configdesc/fcitx-pinyin-enhance.desc
%{_datadir}/%{name}/addon/fcitx-pinyin.conf
%{_datadir}/%{name}/addon/fcitx-pinyin-enhance.conf
%{_datadir}/%{name}/imicon/pinyin.png
%{_datadir}/%{name}/imicon/shuangpin.png
%{_libdir}/%{name}/%{name}-pinyin.so
%{_libdir}/%{name}/%{name}-pinyin-enhance.so
%{_datadir}/%{name}/py-enhance/

%files qw
%license COPYING
%doc
%{_datadir}/%{name}/inputmethod/qw.conf
%{_libdir}/%{name}/%{name}-qw.so
%{_datadir}/%{name}/addon/fcitx-qw.conf

%files table
%license COPYING
%doc
%{_datadir}/%{name}/configdesc/table.desc
%{_libdir}/%{name}/%{name}-table.so
%{_datadir}/%{name}/addon/fcitx-table.conf

%files gtk2
%license COPYING
%{_libdir}/gtk-2.0/%{gtk2_binary_version}/immodules/im-fcitx.so

%files gtk3
%license COPYING
%{_libdir}/gtk-3.0/%{gtk3_binary_version}/immodules/im-fcitx.so

#%%if 0%%{?rhel} < 8
#%files qt4
#%%{_libdir}/qt4/plugins/inputmethods/qtim-fcitx.so
#%%endif

%changelog
* Fri Mar 20 2020 Robin Lee <cheeselee@fedoraproject.org> - 4.2.9.7-3
- Add switches for el8

* Tue Jan 28 2020 Fedora Release Engineering <releng@fedoraproject.org> - 4.2.9.7-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_32_Mass_Rebuild

* Sun Dec  1 2019 Robin Lee <cheeselee@fedoraproject.org> - 4.2.9.7-1
- Release 4.2.9.7

* Thu Jul 25 2019 Fedora Release Engineering <releng@fedoraproject.org> - 4.2.9.6-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Thu Jun 20 2019 Robin Lee <cheeselee@fedoraproject.org> - 4.2.9.6-5
- BR: add libuuid-devel

* Thu Jan 31 2019 Fedora Release Engineering <releng@fedoraproject.org> - 4.2.9.6-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Fri Sep 21 2018 Jan Grulich <jgrulich@redhat.com> - 4.2.9.6-3
- rebuild (qt5)

* Fri Jul 13 2018 Fedora Release Engineering <releng@fedoraproject.org> - 4.2.9.6-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Sun Apr  1 2018 Robin Lee <cheeselee@fedoraproject.org> - 4.2.9.6-1
- Update to 4.2.9.6

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 4.2.9.5-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Thu Feb  1 2018 Robin Lee <cheeselee@fedoraproject.org> - 4.2.9.5-1
- Update to 4.2.9.5

* Sat Jan 06 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 4.2.9.4-3
- Remove obsolete scriptlets

* Sat Jan 06 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 4.2.9.4-2
- Remove obsolete scriptlets

* Thu Sep 28 2017 Robin Lee <cheeselee@fedoraproject.org> - 4.2.9.4-1
- Update to 4.2.9.4 (BZ#1496438)

* Sat Sep 23 2017 Robin Lee <cheeselee@fedoraproject> - 4.2.9.3-1
- Update to 4.2.9.3 (BZ#1487968)

* Wed Aug 02 2017 Fedora Release Engineering <releng@fedoraproject.org> - 4.2.9.1-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Binutils_Mass_Rebuild

* Wed Jul 26 2017 Fedora Release Engineering <releng@fedoraproject.org> - 4.2.9.1-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Fri Feb 10 2017 Fedora Release Engineering <releng@fedoraproject.org> - 4.2.9.1-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Wed Feb 03 2016 Fedora Release Engineering <releng@fedoraproject.org> - 4.2.9.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Mon Jan 25 2016 Robin Lee <cheeselee@fedoraproject.org> - 4.2.9.1-1
- Update to 4.2.9.1

* Wed Sep 30 2015 Robin Lee <cheeselee@fedoraproject.org> - 4.2.9-1
- Update to 4.2.9
- Qt5 im module detected in xinputrc
- BR: gcc-c++, extra-cmake-modules

* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.2.8.6-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Sat Apr 25 2015 Liang Suilong <liangsuilong@gmail.com> - 4.2.8.6-1
- Update to 4.2.8.6

* Mon Jan 26 2015 David Tardon <dtardon@redhat.com> - 4.2.8.4-7
- rebuild for ICU 54.1

* Tue Sep 09 2014 Rex Dieter <rdieter@fedoraproject.org> 4.2.8.4-6
- update scriptlets (mime mostly), tighten subpkg deps, BR: qt4-devel

* Tue Aug 26 2014 David Tardon <dtardon@redhat.com> - 4.2.8.4-5
- rebuild for ICU 53.1

* Sat Aug 16 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.2.8.4-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_22_Mass_Rebuild

* Tue Jul 22 2014 Kalev Lember <kalevlember@gmail.com> - 4.2.8.4-3
- Rebuilt for gobject-introspection 1.41.4

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.2.8.4-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Sat May 24 2014 Robin Lee <cheeselee@fedoraproject.org> - 4.2.8.4-1
- Update to 4.2.8.4

* Fri Feb 14 2014 Parag Nemade <paragn AT fedoraproject DOT org> - 4.2.8.3-2
- Rebuild for icu 52

* Tue Oct 29 2013 Robin Lee <cheeselee@fedoraproject.org> - 4.2.8.3-1
- Update to 4.2.8.3
- Update summary of fcitx package
- Other minor spell fixes

* Fri Aug 23 2013 Robin Lee <cheeselee@fedoraproject.org> - 4.2.8-3
- Own some missed directories
- Update URL's and Source0 URL
- Revise description following upstream wiki

* Sat Aug 03 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.2.8-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Tue Jul  2 2013 Robin Lee <cheeselee@fedoraproject.org> - 4.2.8-1
- Update to 4.2.8 (https://www.csslayer.info/wordpress/fcitx-dev/fcitx-4-2-8/)
- Add scriptlets to update icon cache (BZ#980309)

* Fri Jun 21 2013 Robin Lee <cheeselee@fedoraproject.org> - 4.2.7-6
- Move fcitx4-config to devel package and patch it to use pkg-config to solve
  libdir
- devel subpackage explicitly requires pkgconfig

* Fri Jun 21 2013 Robin Lee <cheeselee@fedoraproject.org> - 4.2.7-5
- Move fcitx4-config to base package to solve multiarch devel subpackage conflict

* Wed Jun 19 2013 Robin Lee <cheeselee@fedoraproject.org> - 4.2.7-4
- BR: lua-devel (BZ#974729)
- Move %%{_datadir}/gir-1.0/Fcitx-1.0.gir %%{_bindir}/fcitx4-config to devel
  subpackage (BZ#965914)
- Co-own %%{_datadir}/gir-1.0/, %%{_libdir}/girepository-1.0/
- Own %%{_libdir}/%%{name}/qt/, %%{_libdir}/%%{name}/
- Other minor cleanup

* Mon Apr 29 2013 Robin Lee <cheeselee@fedoraproject.org> - 4.2.7-3
- Fix gtk2 subpackage description (#830377)

* Sat Mar 23 2013 Liang Suilong <liangsuilong@gmail.com> - 4.2.7-2
- Fix to enable Lua support

* Fri Feb 01 2013 Liang Suilong <liangsuilong@gmail.com> - 4.2.7-1
- Upstream to fcitx-4.2.7

* Sat Jan 26 2013 Kevin Fenzi <kevin@scrye.com> 4.2.6.1-3
- Rebuild for new icu

* Mon Nov 26 2012 Liang Suilong <liangsuilong@gmail.com> - 4.2.6.1-2
- Disable xdg-autostart

* Wed Oct 31 2012 Liang Suilong <liangsuilong@gmail.com> - 4.2.6.1-1
- Upstream to fcitx-4.2.6.1

* Sun Jul 22 2012 Liang Suilong <liangsuilong@gmail.com> - 4.2.5-1
- Upstream to fcitx-4.2.5

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.2.4-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Thu Jun 07 2012 Liang Suilong <liangsuilong@gmail.com> - 4.2.4-2
- Drop fcitx-keyboard
- Divide Table Engine into fcitx-table
- Move GIR Binding into fcitx-libs

* Tue Jun 05 2012 Liang Suilong <liangsuilong@gmail.com> - 4.2.4-1
- Upgrade to fcitx-4.2.4
- Fix the ownership conflict on fcitx and fcitx-data
- Divide Pinyin engine into fcitx-pinyin
- Divide Quwei engine into fcitx-qw
- Divide XKB integration into fcitx-keyboard

* Mon May 07 2012 Liang Suilong <liangsuilong@gmail.com> - 4.2.3-1
- Upgrade to fcitx-4.2.3

* Thu Apr 26 2012 Liang Suilong <liangsuilong@gmail.com> - 4.2.2-2
- Upgrade to fcitx-4.2.2

* Sun Apr 22 2012 Liang Suilong <liangsuilong@gmail.com> - 4.2.2-1
- Upgrade to fcitx-4.2.2

* Sat Feb 04 2012 Liang Suilong <liangsuilong@gmail.com> - 4.2.0-1
- Upgrade to fcitx-4.2.0

* Sun Dec 25 2011 Liang Suilong <liangsuilong@gmail.com> - 4.1.2-3
- Fix the spec

* Sun Dec 25 2011 Liang Suilong <liangsuilong@gmail.com> - 4.1.2-2
- Fix the spec

* Sun Dec 25 2011 Liang Suilong <liangsuilong@gmail.com> - 4.1.2-1
- Update to 4.1.2
- move fcitx4-config to devel

* Fri Sep 09 2011 Liang Suilong <liangsuilong@gmail.com> - 4.1.1-2
- Update xinput-fcitx
- Add fcitx-gtk3 as fcitx requires

* Fri Sep 09 2011 Liang Suilong <liangsuilong@gmail.com> - 4.1.1-1
- Upstream to fcitx-4.1.1

* Fri Sep 09 2011 Liang Suilong <liangsuilong@gmail.com> - 4.1.0-1
- Upstream to fcitx-4.1.0
- Add fcitx-gtk2 as FCITX im module for gtk2
- Add fcitx-gtk3 as FCITX im module for gtk3
- Add fcitx-qt4 as FCITX im module for qt4

* Tue Aug 02 2011 Liang Suilong <liangsuilong@gmail.com> - 4.0.1-5
- Fix that %%files lists a wrong address
- Separate fcitx-libs again

* Tue Aug 02 2011 Liang Suilong <liangsuilong@gmail.com> - 4.0.1-4
- Separates varieties of tables from FCITX
- Merge fcitx-libs into fcitx 

* Sun Jul 03 2011 Liang Suilong <liangsuilong@gmail.com> - 4.0.1-3
- Support GNOME 3 tray icon
- Fix that main window is covered by GNOME Shell 

* Tue Feb 08 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 4.0.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Fri Nov 19 2010 Chen Lei <supercyper@163.com> - 4.0.0-1
- Update to 4.0.0

* Mon Jun 14 2010 Chen Lei <supercyper@163.com> - 3.6.3-5.20100514svn_utf8
- Remove BR:libXext-devel

* Fri May 14 2010 Chen Lei <supercyper@163.com> - 3.6.3-4.20100514svn_utf8
- svn 365

* Sun Apr 18 2010 Chen Lei <supercyper@163.com> - 3.6.3-3.20100410svn_utf8
- Exclude xpm files

* Sat Apr 17 2010 Chen Lei <supercyper@163.com> - 3.6.3-2.20100410svn_utf8
- Update License tag
- Add more explanation for UTF-8 branch

* Mon Apr 12 2010 Chen Lei <supercyper@163.com> - 3.6.3-1.20100410svn_utf8
- Initial Package

