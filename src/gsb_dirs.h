#ifndef _GSB_DIRS_H
#define _GSB_DIRS_H

#include <glib.h>
#include "structures.h"

void gsb_dirs_init ( void );
void gsb_dirs_shutdown ( void );
const gchar *gsb_dirs_get_locale_dir ( void );
const gchar *gsb_dirs_get_pixmaps_dir ( void );
const gchar *gsb_dirs_get_plugins_dir ( void );

/* code from gsb_file_config.h */

/* mise en conformité avec les recommandations FreeDesktop. */

#ifndef _WIN32
#if IS_DEVELOPMENT_VERSION == 1
#define C_GRISBIRC  (g_strconcat ( "/", PACKAGE, "dev.conf", NULL))
#else
#define C_GRISBIRC  (g_strconcat ( "/", PACKAGE, ".conf", NULL))
#endif
#define C_OLD_GRISBIRC  (g_strconcat ( "/.", PACKAGE, "rc", NULL))

#ifdef OS_OSX
#define C_PATH_CONFIG (g_strconcat (g_get_home_dir ( ), G_DIR_SEPARATOR_S,\
                        "Library/Application Support/Grisbi/config", NULL))
#define C_PATH_DATA_FILES (g_strconcat (g_get_home_dir ( ), G_DIR_SEPARATOR_S,\
                        "Library/Application Support/Grisbi/data", NULL))
#else
#define C_PATH_CONFIG (g_strconcat (g_get_user_config_dir ( ), G_DIR_SEPARATOR_S,\
                        "grisbi", NULL))
#define C_PATH_DATA_FILES (g_strconcat (g_get_user_data_dir ( ), G_DIR_SEPARATOR_S,\
                        "grisbi", NULL))
#endif
#else
/* Some old Windows version have difficulties with dat starting file names */
#define C_GRISBIRC  (g_strconcat ( "\\", PACKAGE, "rc", NULL))
#define C_OLD_GRISBIRC  (g_strconcat ( "\\.", PACKAGE, "rc", NULL))
#define C_PATH_CONFIG (win32_get_grisbirc_folder_path( ))
#define C_PATH_DATA_FILES (g_get_home_dir ( ))
#endif

#define C_PATH_CONFIG_ACCELS (g_strconcat ( C_PATH_CONFIG, G_DIR_SEPARATOR_S,\
                        "grisbi-accels", NULL ))

/* code from utils_files.[ch] */

gchar* my_get_XDG_grisbi_data_dir ( void );
gchar* my_get_XDG_grisbirc_dir ( void );
gchar* my_get_grisbirc_dir ( void );
gchar* my_get_gsb_file_default_dir ( void );

#endif
