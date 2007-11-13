/* fichier d'en tête include.h */
#ifndef _INCLUDE_H
#define _INCLUDE_H (1)


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


/* constante de debug originale, obsolete il faudrait utiliser maintenant */
/* la variable globale debugging_grisbi, voir erreur.c */
#define DEBUG utils_str_atoi (getenv ("DEBUG_GRISBI"))

/* constantes definissant le niveau de debug */
#define	DEBUG_LEVEL_ALERT			1		/* grave probleme */
#define DEBUG_LEVEL_IMPORTANT			2		/* probleme moins grave */
#define DEBUG_LEVEL_NOTICE			3		/* probleme encore moins grave :) */
#define DEBUG_LEVEL_INFO			4		/* autre information */
#define DEBUG_LEVEL_DEBUG			5		/* information de debug */
#define MAX_DEBUG_LEVEL				5		/* ignore higher debug levels */

#define devel_debug(x) debug_message("Debug",__FILE__,__LINE__,__PRETTY_FUNCTION__,x,DEBUG_LEVEL_DEBUG,FALSE)
#define notice_debug(x) debug_message("Info",__FILE__,__LINE__,__PRETTY_FUNCTION__,x,DEBUG_LEVEL_INFO,FALSE)
#define warning_debug(x) debug_message("Warning",__FILE__,__LINE__,__PRETTY_FUNCTION__,x,DEBUG_LEVEL_NOTICE,FALSE)
#define important_debug(x) debug_message("Important",__FILE__,__LINE__,__PRETTY_FUNCTION__,x,DEBUG_LEVEL_IMPORTANT,FALSE)
#define alert_debug(x) debug_message("Alert",__FILE__,__LINE__,__PRETTY_FUNCTION__,x,DEBUG_LEVEL_ALERT,FALSE)
#define DEBUG_WHERE_AM_I g_strdup_printf("%s line %05d",__FILE__,__LINE__)

#endif /* _INCLUDE_H */
