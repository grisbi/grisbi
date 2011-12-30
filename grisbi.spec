%define name    grisbi
%define version 0.8.9
%define release %mkrel 1

%define _disable_ld_no_undefined 1
%define _disable_ld_as_needed 1

Summary:        Personal finance manager
Name:           %{name}
Version:        %{version}
Release:        %{release}
License:        GPLv2
Url:            http://www.grisbi.org/
Source0:        http://switch.dl.sourceforge.net/sourceforge/grisbi/%{name}-%{version}.tar.bz2

Group:          Office
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-buildroot
BuildRequires:  libgdk_pixbuf2.0-devel libofx-devel
BuildRequires:  glib2-devel >= 2.0.0 gtk2-devel
BuildRequires:  gettext-devel openssl-devel libpcre-devel

%description
Grisbi helps you to manage your personal finances with Linux.

%prep
%setup -q

%build
%configure
%make

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall_std

mkdir -p $RPM_BUILD_ROOT%_datadir/applications
desktop-file-install --vendor mandriva \
    --dir $RPM_BUILD_ROOT%{_datadir}/applications \
    --add-category "X-MandrivaLinux-MoreApplications-Finances" \
    --add-mime-type=application/x-grisbi \
    --rebuild-mime-info-cache \
    --delete-original \
    $RPM_BUILD_ROOT%_datadir/applications/%{name}.desktop

mkdir -p $RPM_BUILD_ROOT%_datadir/icons/hicolor/48x48/apps
cp -f $RPM_BUILD_ROOT%{_datadir}/pixmaps/grisbi/grisbi.png \
    $RPM_BUILD_ROOT%{_datadir}/icons/hicolor/48x48/apps/grisbi.png

%find_lang %{name} %{name}-tips

%clean
rm -rf $RPM_BUILD_ROOT

%files -f %name.lang
%defattr(-,root,root,0755)
%doc AUTHORS NEWS README
%_bindir/*
%_libdir/grisbi
%dir %_datadir/grisbi
%_datadir/grisbi/categories
%_datadir/pixmaps/*
%_datadir/applications/*
%_mandir/man1/*
%_datadir/mime-info/*
%_iconsdir/hicolor/48x48/apps/%name.png

%changelog
* Fri Dec 30 2011 Pierre Biava <grisbi@pierre.biava.name> 0.8.8-1mdv2010.2
- new version 0.8.9

* Sat Aug 06 2011 Pierre Biava <grisbi@pierre.biava.name> 0.8.8-1mdv2010.2
- new version 0.8.8

* Sun May 22 2011 Pierre Biava <grisbi@pierre.biava.name> 0.8.6-1mdv2010.2
- new version 0.8.6

* Wed Apr 13 2011 Pierre Biava <grisbi@pierre.biava.name> 0.8.5-1mdv2010.1
- new version 0.8.5

* Mon Feb 22 2011 Pierre Biava <grisbi@pierre.biava.name> 0.8.2-1mdv2010.1
- new version 0.8.2

* Sun Feb 20 2011 Pierre Biava <grisbi@pierre.biava.name> 0.8.0-1mdv2010.1
- new version 0.8.0

* Sun Sep 12 2010 Pierre Biava <grisbi@pierre.biava.name> 0.7.4-1mdv2010.1
- new version 0.7.4

* Sun Apr 18 2010 Pierre Biava <grisbi@pierre.biava.name> 0.6.0-1mdv2010.0
- new version 0.6.0

* Sun Aug 30 2009 Pierre Biava <pierre@pierre.biava.name> 0.6.0b1-1mdv2009.0
- 0.6.0b1

* Sun Jan 25 2009 Pierre Biava <pierre@pierre.biava.name> 0.6.0a1-1mdv2009.0
- new version of the spec file for grisbi 0.6.0

* Thu Aug 07 2008 Thierry Vignaud <tvignaud@mandriva.com> 0.5.9-3mdv2009.0
+ Revision: 266978
- rebuild early 2009.0 package (before pixel changes)

  + Pixel <pixel@mandriva.com>
    - rpm filetriggers deprecates update_menus/update_scrollkeeper/update_mime_database/update_icon_cache/update_desktop_database/post_install_gconf_schemas

* Mon May 26 2008 Frederic Crozat <fcrozat@mandriva.com> 0.5.9-2mdv2009.0
+ Revision: 211409
- Patch2: fix build when as-needed is used
- Patch3: fix help path
- Fix buildrequires

  + Thierry Vignaud <tvignaud@mandriva.com>
    - drop old menu
    - kill re-definition of %%buildroot on Pixel's request
    - kill desktop-file-validate's 'warning: key "Encoding" in group "Desktop Entry" is deprecated'

  + Olivier Blin <oblin@mandriva.com>
    - restore BuildRoot

* Wed Mar 21 2007 Adam Williamson <awilliamson@mandriva.com> 0.5.9-2mdv2007.1
+ Revision: 147387
- rebuild for new libofx
- spellcheck summary and description

* Wed Nov 22 2006 JÃ©rÃŽme Soyer <saispo@mandriva.org> 0.5.9-1mdv2007.1
+ Revision: 86320
- Fix Specs
- Add BuildRequires Tetex
- New release 0.5.9
- Import grisbi

* Fri Apr 07 2006 Frederic Crozat <fcrozat@mandriva.com> 0.5.8-2mdk
- Patch0: fix web browser name
- Patch1: fix doc build
- package french manual
- Add requires on latex, needed for printing
- fix buildrequires

* Thu Jan 19 2006 Lenny Cartier <lenny@mandriva.com> 0.5.8-1mdk
- 0.5.8

* Thu Nov 24 2005 Lenny Cartier <lenny@mandriva.com> 0.5.7-3mdk
- rebuild

* Tue Aug 09 2005 Per Ãyvind Karlsen <pkarlsen@mandriva.com> 0.5.7-2mdk
- %%mkrel
- get rid of bizarre stuff
- don't bzip2 icons
- cleanups!

* Fri Jun 10 2005 Lenny Cartier <lenny@mandriva.com> 0.5.7-1mdk
- 0.5.7

* Thu Jan 13 2005 Jerome Soyer <saispo@mandrake.org> 0.5.5-1mdk
- 0.5.5

* Mon Dec 20 2004 Götz Waschk <waschk@linux-mandrake.com> 0.5.3-2mdk
- rebuild for new ofx

* Thu Dec 02 2004 Lenny Cartier <lenny@mandrakesoft.com> 0.5.3-1mdk
- 0.5.3

* Fri Oct 22 2004 Jerome Soyer <saispo@mandrake.org> 0.5.2-1mdk
- 0.5.2
- Update BuildRequires

* Mon Aug 30 2004 Jerome Soyer <saispo@mandrake.org> 0.5.1-3mdk
- fix buildrequires

* Sun Aug 22 2004 Jerome Soyer <saispo@mandrake.org> 0.5.1-2mdk
- fix menu entry

* Thu Aug 05 2004 Lenny Cartier <lenny@mandrakesoft.com> 0.5.1-1mdk
- 0.5.1

* Tue Jul 20 2004 Lenny Cartier <lenny@mandrakesoft.com> 0.5.0-1mdk
- from neoclust <n1c0l4s.l3@wanadoo.fr> :
        - 0.5.0

* Sat Apr 24 2004 Lenny Cartier <lenny@mandrakesoft.com> 0.4.5-1mdk
- 0.4.5
- merge with original grisbi spec
