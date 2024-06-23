/* Defined if app is GtkOSXApplication */
#define GTKOSXAPPLICATION 1

/* Gettext package. */
#define GETTEXT_PACKAGE "grisbi"

/* Define to 1 if you have the `backtrace' function. */
#define HAVE_BACKTRACE 1

/* Define if goffice support is enabled */
#define HAVE_GOFFICE 1

/* Build with ofx support */
#define HAVE_OFX 1

/* Define if OpenSSL support is enabled */
#define HAVE_SSL 1

/* Define if libxml2 support is enabled */
#define HAVE_XML2 1

/* Defined if Grisbi is a developmeent version */
#define DEBUG 1

/* Defined if system is Mac OS X */
#define OS_OSX 1

/* Name of package */
#define PACKAGE "grisbi"

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif


/* Version number of package */
#define VERSION "1.9.0"
#define GIT_HASH "Xcode-build"

#define HELP_PATH ""
#define LOGO_PATH ""

#define DATA_PATH "/usr/local/share/grisbi/"
#define LOCALEDIR "/usr/local/share/locale"
#define PIXMAPS_DIR "/usr/local/share/pixmaps/grisbi"
#define UI_DIR ""

#define USE_CONFIG_FILE
