#ifndef _GSB_FILE_CONFIG_H
#define _GSB_FILE_CONFIG_H (1)

#include <glib.h>

#define ETAT_WWW_BROWSER "xdg-open"

/* START_DECLARATION */
gboolean gsb_file_config_load_app_config (void);
gboolean gsb_file_config_save_app_config (void);
/* END_DECLARATION */

#endif
