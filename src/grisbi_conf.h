#ifdef USE_CONFIG_FILE

#ifndef _GSB_FILE_CONFIG_H
#define _GSB_FILE_CONFIG_H (1)

#include <glib.h>

/* START_DECLARATION */
gboolean grisbi_conf_load_app_config (void);
gboolean grisbi_conf_save_app_config (void);
/* END_DECLARATION */

#endif
#endif /* USE_CONFIG_FILE */
