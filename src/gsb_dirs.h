#ifndef _GSB_DIRS_H
#define _GSB_DIRS_H

#include <glib.h>

void gsb_dirs_init ( void );
void gsb_dirs_shutdown ( void );
const gchar *gsb_dirs_get_categories_dir ( void );
const gchar *gsb_dirs_get_locale_dir ( void );
const gchar *gsb_dirs_get_pixmaps_dir ( void );
const gchar *gsb_dirs_get_plugins_dir ( void );
const gchar *gsb_dirs_get_ui_dir ( void );
const gchar *C_GRISBIRC ( void );
const gchar *C_OLD_GRISBIRC ( void );
const gchar *C_PATH_CONFIG ( void );
const gchar *C_PATH_DATA_FILES ( void );
const gchar *C_PATH_CONFIG_ACCELS ( void );
const gchar *my_get_XDG_grisbi_data_dir ( void );
const gchar *my_get_XDG_grisbirc_dir ( void );
const gchar *my_get_grisbirc_dir ( void );
const gchar *my_get_gsb_file_default_dir ( void );

#endif
