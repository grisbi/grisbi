#ifndef _GSB_FILE_CONFIG_H
#define _GSB_FILE_CONFIG_H (1)

#define ETAT_WWW_BROWSER "www-browser"

#ifndef _WIN32
#define C_DIRECTORY_SEPARATOR "/"
#define C_GRISBIRC  "/.grisbirc"
#else
/* Some old Windows version have difficulties with dat starting file names */
#define C_DIRECTORY_SEPARATOR "\\"
#define C_GRISBIRC  "\\grisbi.rc"
#endif


/* START_DECLARATION */
gboolean gsb_file_config_load_config ( void );
gboolean gsb_file_config_save_config ( void );
/* END_DECLARATION */

#endif
