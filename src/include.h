/* fichier d'en tête include.h */
#ifndef _INCLUDE_H
#define _INCLUDE_H (1)


#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STRINGS_H
#  include <strings.h>
#endif
#ifdef HAVE_STRING_H
#  include <string.h>
#endif
#include <sys/types.h>
#include <sys/timeb.h>
#include <time.h>
#include <sys/stat.h>
#ifndef _MSC_VER
#	include <unistd.h>
#	include <pwd.h>
#endif /*_MSC_VER */
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>

#include <gtk/gtktextview.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gdk-pixbuf/gdk-pixdata.h>

#include <glib/gi18n.h>

/* The following lines revert back to FPRINTF on Windows Systems(LIBINTL_FPRINTF seems BUGGY) */
#ifdef _WIN32
# undef fprintf
#endif

/* g_fopen returns an incompatible pointer on windows with MSVC. This is a fix*/
#ifdef _MSC_VER
#undef g_fopen
#define g_fopen fopen
#endif

#endif /* _INCLUDE_H */
