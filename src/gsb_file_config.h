#ifndef _GSB_FILE_CONFIG_H
#define _GSB_FILE_CONFIG_H (1)

#include <glib.h>

/*START_INCLUDE*/
#include "structures.h"
/*END_INCLUDE*/

#define ETAT_WWW_BROWSER "xdg-open"

/* START_DECLARATION */
gboolean gsb_file_config_load_config ( GrisbiAppConf *conf );
gboolean gsb_file_config_save_config ( GrisbiAppConf *conf );
const gchar *gsb_config_get_conf_filename ( void );
void gsb_config_free_conf_filename ( void );
void gsb_config_initialise_conf_filename ( void );
/* END_DECLARATION */

#endif
