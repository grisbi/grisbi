/* fichier d'en tête include.h */


#include "config.h"
#include <libintl.h>
#include <locale.h>
#include <stdio.h>
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
#include "gtkcombofix.h"

#include <libxml/encoding.h>
#include <libxml/tree.h>

#define GTK_ENABLE_BROKEN 1
#define G_BROKEN_FILENAMES 1
#include <gtk/gtktext.h>
#include <gdk/gdkkeysyms.h>

