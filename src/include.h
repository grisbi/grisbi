/* fichier d'en tête include.h */


#include "config.h"
#include <libintl.h>
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
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>

#define GTK_ENABLE_BROKEN 1
#define G_BROKEN_FILENAMES 1
#include <gtk/gtktext.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>



/* to avoid an error because of mk_include and g2banking */
#ifndef HAVE_G2BANKING
#define AB_BANKING void
#endif


/* Define gettext functions */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  define _(String) gettext(String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop(String)
#  else
#    define N_(String) (String)
#  endif /* gettext_noop */
#else
#  define _(String) (String)
#  define N_(String) (String)
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,String) (String)
#  define dcgettext(Domain,String,Type) (String)
#  define bindtextdomain(Domain,Directory) (Domain) 
#endif /* ENABLE_NLS */
