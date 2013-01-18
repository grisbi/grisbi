#ifndef _GSB_DIRS_H
#define _GSB_DIRS_H

#include <glib.h>

void gsb_dirs_init ( char* gsb_bin_path );
void gsb_dirs_shutdown ( void );
const gchar *gsb_dirs_get_categories_dir ( void );
const gchar *gsb_dirs_get_default_dir ( void );
const gchar *gsb_dirs_get_locale_dir ( void );
const gchar *gsb_dirs_get_pixmaps_dir ( void );
const gchar *gsb_dirs_get_ui_dir ( void );
const gchar *gsb_dirs_get_user_config_dir ( void );
const gchar *gsb_dirs_get_user_data_dir ( void );

const gchar *gsb_dirs_get_accelerator_filename ( void );
const gchar *gsb_dirs_get_grisbirc_filename ( void );
#endif
