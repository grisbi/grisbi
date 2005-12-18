#ifndef _GSB_FILE_CONFIG_H
#define _GSB_FILE_CONFIG_H (1)

/* START_INCLUDE_H */
#include "gsb_file_config.h"
/* END_INCLUDE_H */


#define ETAT_WWW_BROWSER "www-browser"

#ifndef _WIN32
#define C_DIRECTORY_SEPARATOR "/"
#define C_GRISBIRC  (g_strconcat ( "/.", PACKAGE, "rc", NULL))
#else
/* Some old Windows version have difficulties with dat starting file names */
#define C_DIRECTORY_SEPARATOR "\\"
#define C_GRISBIRC  (g_strconcat ( "\\", PACKAGE, "rc", NULL))
#endif


/* START_DECLARATION */
gboolean gsb_file_config_load_config ( void );
gboolean gsb_file_config_save_config ( void );
/* END_DECLARATION */

#endif
