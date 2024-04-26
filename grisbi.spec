#-----------------------------------------------------------------------------
%{?!dist:	%define	dist	.ok_9.1}
#-----------------------------------------------------------------------------
Name		:	grisbi
Version		:	1.9.96
Release		:	1.96.12%{?dist}
Summary		:	Personal finances manager
Group		:	Application

Packager	:	Osukiss Project	<build-ok@osukiss.org>

License		:	GPLv2+
URL		:	http://www.grisbi.org
			#https://sourceforge.net/projects/grisbi/files/stable

Source0		:	%{name}-%{version}.tar.bz2
#-----------------------------------------------------------------------------
BuildRequires	:	libgsf
BuildRequires	:	glib
BuildRequires	:	goffice-devel
BuildRequires	:	gtk3+
#-----------------------------------------------------------------------------
Requires	:	glib
Requires	:	desktop-file-utils
Requires	:	hicolor-icon-theme
Requires	:	gtk3+
Requires	:	shared-mime-info

Requires(post)	:	%{_bindir}/update-mime-database
Requires(post)	:	%{_bindir}/gtk-update-icon-cache
Requires(post)	:	%{_bindir}/update-desktop-database
Requires(post)	:	%{_bindir}/glib-compile-schemas
#-----------------------------------------------------------------------------
%description
%{name} is a very functional personal financial management program
with a lot of features: checking, cash and liabilities accounts,
several accounts with automatic contra entries, several currencies,
including euro, arbitrary currency for every operation, money
interchange fees, switch to euro account per account, description
of the transactions with third parties, categories, sub-categories,
financial year, notes, breakdown, transfers between accounts, even
for accounts of different currencies, bank reconciliation, scheduled
transactions, automatic recall of last transaction for every third
party, nice and easy user interface, user manual, QIF import/export.

%prep
%setup -q

#---------------------------------------------------------
#building application
%build

autoreconf -vif
./configure						\
        --prefix=%{_prefix}				\
        --includedir=%{_includedir}			\
	--exec-prefix=%{_exec_prefix}			\
	--libexecdir=%{_libexecdir}			\
        --sysconfdir=%{_sysconfdir}			\
        --sbindir=%{_sbindir}				\
	--mandir=%{_mandir}				\
	--libdir=%{_libdir}				\
	--infodir=%{_infodir}				\
	--docdir=%{_defaultdocdir}/%{name}-%{version}	\

%define _smp_mflags -j%(/usr/bin/getconf _NPROCESSORS_ONLN)
make %{?_smp_mflags}

%clean
%{__rm} -rf %{buildroot}

#installing program
%install

%{__rm} -rf %{buildroot}

%{__make}						\
	DESTDIR=%{buildroot}				\
	install

%files			
%defattr(-,root,root,-)

%doc AUTHORS COPYING NEWS README
%{_bindir}/%{name}
%{_datadir}/doc/%{name}-%{version}/C/*.{gif,html,dat}
%{_datadir}/doc/%{name}-%{version}/de/*
%{_datadir}/doc/%{name}-%{version}/en/*
%{_datadir}/doc/%{name}-%{version}/fr/*
%{_datadir}/applications/org.grisbi.Grisbi.desktop
%{_datadir}/glib-2.0/schemas/org.gtk.%{name}.gschema.xml
%{_datadir}/mime/packages/%{name}.xml
%{_datadir}/%{name}/ui/%{name}-*.css
%{_datadir}/%{name}/ui/%{name}.css
%{_datadir}/%{name}/categories/*
%{_datadir}/pixmaps/%{name}/flags/*.png
%{_datadir}/pixmaps/%{name}/*.png
%{_datadir}/pixmaps/%{name}/%{name}.svg
%{_datadir}/icons/hicolor/scalable/apps/%{name}.svg
%{_datadir}/locale/*/LC_MESSAGES/%{name}.mo
%{_datadir}/mime-info/%{name}.*
%{_mandir}/man1/%{name}.1.gz

%post
%{_bindir}/gtk-update-icon-cache -t %{_datadir}/icons/hicolor  2> /dev/null
%{_bindir}/update-desktop-database 2> /dev/null
%{_bindir}/glib-compile-schemas %{_datadir}/glib-2.0/schemas 2> /dev/null
%{_bindir}/update-mime-database -n %{_datadir}/mime 2> /dev/null

%postun
if [ "$1" = 0 ]; then
  mkdir -p %{_datadir}/icons/hicolor &> /dev/null
  %{_bindir}/update-mime-database -n %{_datadir}/mime 2> /dev/null
  %{_bindir}/gtk-update-icon-cache -t %{_datadir}/icons/hicolor 2> /dev/null
  %{_bindir}/glib-compile-schemas %{_datadir}/glib-2.0/schemas/ 2> /dev/null
  %{_bindir}/update-desktop-database 2> /dev/null
  fi

%changelog
