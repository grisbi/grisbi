#ifndef _GSB_DIRS_H
#define _GSB_DIRS_H

#include <glib.h>

void gsb_dirs_init ( void );
void gsb_dirs_shutdown ( void );
const gchar *gsb_dirs_get_locale_dir ( void );
const gchar *gsb_dirs_get_pixmaps_dir ( void );
const gchar *gsb_dirs_get_plugins_dir ( void );

#endif
