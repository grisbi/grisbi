#ifndef _GSB_FILE_CONFIG_H
#define _GSB_FILE_CONFIG_H (1)

/* START_INCLUDE_H */
/* END_INCLUDE_H */


#define ETAT_WWW_BROWSER "firefox"

/* mise en conformité avec les recommandations FreeDesktop. */
#ifndef _WIN32
#define C_GRISBIRC  (g_strconcat ( "/", PACKAGE, ".conf", NULL))
#define C_OLD_GRISBIRC  (g_strconcat ( "/.", PACKAGE, "rc", NULL))
#define C_PATH_CONFIG (g_strconcat (g_get_user_config_dir ( ), G_DIR_SEPARATOR_S,\
                        "grisbi", NULL))
#define C_PATH_DATA_FILES (g_strconcat (g_get_user_data_dir ( ), G_DIR_SEPARATOR_S,\
                        "grisbi", NULL))
#else
/* Some old Windows version have difficulties with dat starting file names */
#define C_GRISBIRC  (g_strconcat ( "\\", PACKAGE, "rc", NULL))
#endif


/* START_DECLARATION */
gboolean gsb_file_config_load_config ( void );
gboolean gsb_file_config_save_config ( void );
/* END_DECLARATION */

#endif
