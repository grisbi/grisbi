dnl Process this file with autoconf to produce a configure script.

m4_define(grisbi_major_version, 3)
m4_define(grisbi_minor_version, 1)
m4_define(grisbi_micro_version, 1)
m4_define(grisbi_version, grisbi_major_version.grisbi_minor_version.grisbi_micro_version)

m4_pattern_allow([AM_PROG_AR])

AC_INIT([grisbi],[grisbi_version],[https://www.grisbi.org/mantis/],[grisbi],[https://www.grisbi.org/])

AC_CONFIG_SRCDIR(src/main.c)
AC_CONFIG_AUX_DIR(.)
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_DEFINE(GRISBI_MAJOR_VERSION, grisbi_major_version, [grisbi major version])
AC_SUBST(GRISBI_MAJOR_VERSION, grisbi_major_version)
AC_DEFINE(GRISBI_MINOR_VERSION, grisbi_minor_version, [grisbi minor version])
AC_SUBST(GRISBI_MINOR_VERSION, grisbi_minor_version)
AC_DEFINE(GRISBI_MICRO_VERSION, grisbi_micro_version, [grisbi micro version])
AC_SUBST(GRISBI_MICRO_VERSION, grisbi_micro_version)

# Dependencies
GLIB_REQUIRED=2.56
GTK_REQUIRED=3.24
LIBGSF_REQUIRED=1.14

# Optionnel
LIBGOFFICE_REQUIRED=0.10.0
LIBOFX_REQUIRED=0.9
LIBSSL_REQUIRED=1.0.0
LIBXML_REQUIRED=2.5.0

# last commit for package
grisbi_git_hash=GRISBI_GIT_HASH

dnl ================================================================
dnl Standard checks.
dnl ================================================================

AC_PROG_CC
AC_PROG_OBJC
AC_USE_SYSTEM_EXTENSIONS
AM_PROG_AR

dnl needed on osx
AC_PROG_OBJC

dnl Check if we have backtrace support
AC_CHECK_FUNCS(backtrace)

AM_INIT_AUTOMAKE([-Wall foreign dist-bzip2 no-dist-gzip])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl Initialize libtool
LT_INIT
LT_LIB_M

dnl GLIB_COMPILE_RESOURCES
AC_PATH_PROG(GLIB_COMPILE_RESOURCES, glib-compile-resources)

dnl new warning from ar
AR_FLAGS='cr'
AC_SUBST(AR_FLAGS)

dnl ================================================================
dnl Check for NLS support.
dnl ================================================================

AM_GNU_GETTEXT_VERSION([0.21])
AM_GNU_GETTEXT([external])

GETTEXT_PACKAGE=AC_PACKAGE_NAME
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext package.])

dnl ================================================================
dnl Main Grisbi libraries
dnl ================================================================
PKG_CHECK_MODULES(ZLIB, [zlib], have_zlib_pc=yes, have_zlib_pc=no)
PKG_CHECK_MODULES(GTK, gtk+-3.0 >= $GTK_REQUIRED)
PKG_CHECK_MODULES(GLIB, glib-2.0 >= $GLIB_REQUIRED)
PKG_CHECK_MODULES(LIBGSF, libgsf-1 >= $LIBGSF_REQUIRED)
AS_IF([test "$have_zlib_pc" = "no"],
	[AC_CHECK_LIB([z],[gzwrite])
	 AC_SUBST(GRISBI_LIBS)])

# $LIBM courtesy of LT_LIB_M
GRISBI_LIBS="$LIBM $GRISBI_LIBS"

dnl ================================================================
dnl Defined if Grisbi is a development version
dnl ================================================================

AC_ARG_ENABLE(debug,
	AS_HELP_STRING([--enable-debug],[enable extra debug code]),
	[enable_debug=$enableval],[enable_debug=no])

# make DEBUG = 1 for master and odd minor version
AS_IF([expr ${GRISBI_MINOR_VERSION} % 2 > /dev/null],
	[enable_debug=yes])

AS_IF([test "x$enable_debug" = xyes],
	[AC_DEFINE(DEBUG,1,[Defined if Grisbi is a developmeent version])
	CFLAGS="$CFLAGS -g"
	git_hash=`git rev-parse --short HEAD`
	AS_IF([test -z "$git_hash"],
		[AC_DEFINE_UNQUOTED(GIT_HASH, ["$grisbi_git_hash"], [git head short hash])],
		[AC_DEFINE_UNQUOTED(GIT_HASH, ["$git_hash"], [git head short hash])])
	])

AC_ARG_ENABLE(early-debug,
	AS_HELP_STRING([--enable-early-debug],[enable early debug code in a file]),
	[enable_early_debug=$enableval],[enable_early_debug=no])
AS_IF([test "x$enable_early_debug" = xyes],
	[AC_DEFINE(EARLY_DEBUG,1,[Defined if Grisbi should log to a file])])
AM_CONDITIONAL(EARLY_DEBUG, [test "$enable_early_debug" = yes])

dnl ================================================================
dnl XML2
dnl ================================================================

AC_ARG_WITH(libxml2,
	AS_HELP_STRING([--with-libxml2],[libxml2 support for the GnuCash plugin (default=auto)]),
	[build_libxml2=$withval],
	[build_libxml2=auto])

PKG_CHECK_MODULES(XML2, [libxml-2.0 >= 2.0], have_libxml2=yes, have_libxml2=no)
AS_IF([test "$build_libxml2" = yes -a "$have_libxml2" = no],
	[AC_MSG_ERROR([Libxml2/GnuCash support was requested but libxml2 could not be found.])])
AS_IF([test "$build_libxml2" != no],
	[build_libxml2=$have_libxml2],
	[build_libxml2=no])
AS_IF([test "$build_libxml2" = yes],
	[AC_DEFINE(HAVE_XML2, 1, [Define if libxml2 support is enabled])])
AM_CONDITIONAL(HAVE_XML2, [test "$build_libxml2" = yes])

dnl ================================================================
dnl OpenSSL Check
dnl ================================================================

AC_ARG_WITH(openssl,
	AS_HELP_STRING([--with-openssl],[OpenSSL support for file encryption support (default=auto)]),
	[build_openssl=$withval],
	[build_openssl=auto])

PKG_CHECK_MODULES(OPENSSL, [libcrypto >= $LIBSSL_REQUIRED], have_openssl=yes, have_openssl=no)
AS_IF([test "$build_openssl" = yes -a "$have_openssl" = no],
	[AC_MSG_ERROR([OpenSSL support was requested but libssl could not be found.])])
AS_IF([test "$build_openssl" != no],
	[build_openssl=$have_openssl],
	[build_openssl=no])
AS_IF([test "$build_openssl" = yes],
	[AC_DEFINE(HAVE_SSL, 1, [Define if OpenSSL support is enabled])])
AM_CONDITIONAL(HAVE_SSL, [test "$build_openssl" = yes])

dnl ================================================================
dnl Check for cunit
dnl ================================================================

AC_ARG_WITH(cunit,
	AS_HELP_STRING([--with-cunit],[CUnit support for unit tests (default=no)]),
	[build_cunit=$withval],
	[build_cunit=no])

AC_CHECK_HEADER([CUnit/Basic.h], have_cunit=yes, have_cunit=no)
AS_IF([test "$build_cunit" = yes -a "$have_cunit" = no],
	[AC_MSG_ERROR([CUnit support was requested but headers/library could not be found.])])
AS_IF([test "$build_cunit" != no],
	[build_cunit=$have_cunit],
	[build_cunit=no])
AS_IF([test "$build_cunit" = yes],
	[CUNIT_CFLAGS="-DHAVE_CUNIT"
	CUNIT_LIBS="-lcunit"])
AC_SUBST(CUNIT_CFLAGS)
AC_SUBST(CUNIT_LIBS)
AM_CONDITIONAL(HAVE_CUNIT, [test "$build_cunit" = yes])

dnl ================================================================
dnl Check for libofx
dnl ================================================================

AC_ARG_WITH(ofx,
	AS_HELP_STRING([--with-ofx],[build grisbi with ofx support (default=auto)]),
	[build_ofx=$withval],
	[build_ofx=auto])

PKG_CHECK_MODULES(LIBOFX, [libofx >= $LIBOFX_REQUIRED], have_ofx=yes, have_ofx=no)
AS_IF([test "$build_ofx" = yes -a "$with_ofx" = no],
	[AC_MSG_ERROR([Ofx support was requested but libofx could not be found.])])
AS_IF([test "$build_ofx" != no],
	[build_ofx=$have_ofx],
	[build_ofx=no])
AS_IF([test "$build_ofx" = yes],
	[AC_DEFINE(HAVE_OFX, 1, [Build with ofx support])])
AM_CONDITIONAL(HAVE_OFX, [test "$build_ofx" = yes])

dnl ================================================================
dnl Check for libgoffice library
dnl ================================================================

AC_ARG_WITH(goffice,
	AS_HELP_STRING([--with-goffice],[GOffice support of graphics (default=auto)]),
	[build_goffice=$withval],
	[build_goffice=auto])

PKG_CHECK_MODULES(GOFFICE, [libgoffice-0.10 >= $LIBGOFFICE_REQUIRED], have_goffice=yes, have_goffice=no)
AS_IF([test "$build_goffice" = yes -a "$have_goffice" = no],
	[AC_MSG_ERROR([GOffice support was requested but goffice could not be found.])])
AS_IF([test "$build_goffice" != no],
	[build_goffice=$have_goffice],
	[build_goffice=no])
AS_IF([test "$build_goffice" = yes],
	[AC_DEFINE(HAVE_GOFFICE, 1, [Define if goffice support is enabled])])

AM_CONDITIONAL(HAVE_GOFFICE, [test "$build_goffice" = yes])

dnl ================================================================
dnl WError support
dnl ================================================================

AC_ARG_ENABLE(werror,
 	[AS_HELP_STRING([--enable-werror],
 	[Treates warning as errors (default=no)])],,
 	enable_werror=no)
AS_IF([test "x$enable_werror" = xyes],
	[AS_IF([test "x$GCC" = "xyes"],
		[CFLAGS="$CFLAGS -Wall -Wextra -Wno-deprecated-declarations -Wno-unused-parameter"
		 AC_MSG_WARN([Wall -Wextra and other flags are enabled])],
		[AC_MSG_WARN([Wall -Wextra and other flags can be enabled only with gcc])])])

AC_ARG_ENABLE(real-werror,
 	[AS_HELP_STRING([--enable-real-werror],
 	[Treates warning as errors (default=no)])],,
 	enable_real_werror=no)
AS_IF([test "x$enable_real_werror" = xyes],
	[CFLAGS="$CFLAGS -Werror"
	 AC_MSG_WARN([-Werror flag is enabled])])

dnl ================================================================
dnl NoOptim support
dnl ================================================================

AC_ARG_ENABLE(nooptim,
 	[AS_HELP_STRING([--enable-nooptim],
 	[Outputs a summary of memory usage when grisbi terminates (default=no)])],,
 	enable_nooptim=no)
AS_IF([test "x$enable_nooptim" = xyes],
	[CFLAGS="$CFLAGS -O0"
	AC_MSG_WARN([-O0 is enabled])])
AC_DEFINE_UNQUOTED([GSB_NOOPTIM], [${GSB_NOOPTIM}],
	[define to 1 if g_no_optim support is enabled])
AC_SUBST(GSB_NOOPTIM)

dnl ================================================================
dnl basic check for msys2 platform
dnl ================================================================

AC_MSG_CHECKING([for MSYS2 platform])
AS_CASE([$host_os],
  [*msys*], [MSYS2=yes
  	AC_MSG_RESULT([MSYS2])
  	platform_win=yes],
  [*mingw32*], [MINGW32=yes
  	AC_MSG_RESULT([MINGW32])
  	platform_win=yes],
  [*], [platform_win=no
    AC_MSG_RESULT([$platform_win])]
)

AS_IF([test "$platform_win" = "yes"],
	[NSIS_BITS=64bit
	NSIS_DEFAULTINSTDIR="Program Files"
	AS_IF([test "$MSYSTEM"x == "MINGW32"x],
		[NSIS_BITS=32bit;
		NSIS_DEFAULTINSTDIR="Program Files (x86)"])
	NSIS_SRCDIR=$({ cd ${srcdir} && pwd -W; } | sed 's|/|\\|g')
	NSIS_INSTDIR=$({ mkdir -p ${prefix}; cd ${prefix} && pwd -W; }| sed 's|/|\\|g')

	AC_SUBST(NSIS_BITS)
	AC_SUBST(NSIS_DEFAULTINSTDIR)
	AC_SUBST(NSIS_SRCDIR)
	AC_SUBST(NSIS_INSTDIR)

	AS_IF([expr ${GRISBI_MINOR_VERSION} % 2 > /dev/null],
		[NSIS_DATE=$( date +'-%Y.%m.%d-%H' )],
		[NSIS_DATE=""])
	AC_SUBST(NSIS_DATE)

	TOP_SRCDIR_WINBS=$({ cd ${srcdir} && pwd -W; } | sed 's|\\|\\\\|g')
	AC_SUBST(TOP_SRCDIR_WINBS)])

AM_CONDITIONAL(WIN32, test "$platform_win" = "yes")

dnl ================================================================
dnl check for osx platform
dnl ================================================================

AC_MSG_CHECKING([for Mac OS X platform])
AS_CASE(["$host"],
  [rhapsody* | darwin[1567].*],
    [AC_MSG_RESULT([darwin, but too old])
    AC_MSG_ERROR([This platform is not supported, please update to latest  darwin])],
  [*-*-darwin*], [ platform_osx=yes ],
  [*], [ platform_osx=no ])
AC_MSG_RESULT([$platform_osx])
AM_CONDITIONAL(PLATFORM_OSX, test "$platform_osx" = "yes")
AS_IF([test "x$platform_osx" = xyes],
  [AC_DEFINE([OS_OSX],[1],[Defined if system is Mac OS X])
  dnl check for native osx
  gdk_windowing=`$PKG_CONFIG --variable=targets gdk-3.0`
  AC_MSG_CHECKING([for GDK-Quartz])

  AS_IF([test "x$gdk_windowing" = xquartz],
        [platform=darwin/quartz
         AC_MSG_RESULT(yes)
         AC_DEFINE(GDK_QUARTZ,[1],[Using GDK Quartz (not X11)])

		 PKG_CHECK_MODULES(GTK_MAC, gtk-mac-integration-gtk3, [os_osx=yes], [os_osx=no])
         AS_IF([test "x$os_osx" = xyes],
			[AC_DEFINE([GTKOSXAPPLICATION],[1],[Defined if app is GtkOSXApplication])
			 AC_SUBST(GTK_MAC_LIBS)
			 AC_SUBST(GTK_MAC_CFLAGS)])],
		[AC_MSG_RESULT(no)
		 os_osx=no])
  ],
  [os_osx=no])

dnl ================================================================
dnl french doc
dnl ================================================================

AC_ARG_ENABLE(frenchdoc,
  [AS_HELP_STRING([--disable-frenchdoc],
  [Install french user documentation (default=yes)])],,
  enable_frenchdoc=yes)
AM_CONDITIONAL(FRENCHDOC, test "$enable_frenchdoc" = "yes")


dnl ================================================================
dnl Makefiles
dnl ================================================================

AC_CONFIG_FILES([
Makefile
help/Makefile
help/C/Makefile
help/de/Makefile
help/en/Makefile
help/fr/Makefile
help/fr/image/Makefile
help/fr/image/icon/Makefile
help/fr/image/screenshot/Makefile
pixmaps/Makefile
pixmaps/flags/Makefile
po/Makefile.in
share/Makefile
share/grisbi.nsi
share/categories/Makefile
share/categories/C/Makefile
share/categories/fr/Makefile
share/categories/de/Makefile
share/categories/it/Makefile
share/categories/ru/Makefile
src/Makefile
src/win-main.rc
src/images/Makefile
src/tests/Makefile
src/plugins/gnucash/Makefile
src/plugins/ofx/Makefile
src/plugins/openssl/Makefile
src/etats/Makefile
src/prefs/Makefile
src/widgets/Makefile
src/ui/Makefile
])

AC_OUTPUT

AS_IF([test "x$enable_werror" = "xyes"],
	[echo "
                werror:                    yes"])
echo "

                Build with libxml2:        $build_libxml2
                Build with OpenSSL:        $build_openssl
                Build with OFX plugin:     $build_ofx
                Build with goffice:        $build_goffice
                Build win32:               $platform_win
                Build gtk_mac_integration: $os_osx
                CUnit/unit tests support:  $build_cunit
                Install french user doc:   $enable_frenchdoc
                Enable Debug:              $enable_debug
                Enable Early Debug:        $enable_early_debug
                Enable fail on error:      $enable_real_werror
                Enable nooptim:            $enable_nooptim
                Compiler flags:            $CFLAGS

"
AS_IF([expr ${GRISBI_MINOR_VERSION} % 2 > /dev/null],
[
cat << EOF
 *** IMPORTANT ***

This is an unstable version of Grisbi. It is for test purposes only.
Please, DO NOT use it in a production environment. It will probably
crash and you will lose your data.

If you are looking for a stable release of Grisbi (2.0.x) download it from:
https://sourceforge.net/projects/grisbi/files/grisbi%20stable/

Thanks,
The Grisbi team

 *** END OF WARNING ***
EOF
])
