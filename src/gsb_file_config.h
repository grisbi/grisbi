#ifndef _GSB_FILE_CONFIG_H
#define _GSB_FILE_CONFIG_H (1)

/* START_INCLUDE_H */
#include "structures.h"
/* END_INCLUDE_H */


#define ETAT_WWW_BROWSER "xdg-open"

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

/* START_DECLARATION */
gboolean gsb_file_config_load_config ( void );
gboolean gsb_file_config_save_config ( void );
/* END_DECLARATION */

#endif
