#ifndef GSB_PLUGINS_H
#define GSB_PLUGINS_H

/* START_INCLUDE_H */
#include "gsb_plugins.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gsb_plugin * gsb_find_plugin ( gchar * plugin_name );
gchar * gsb_plugin_get_list ();
void gsb_plugins_scan_dir ( const char *dirname );
/* END_DECLARATION */


typedef struct gsb_plugin {
    gchar * name;
    gboolean (* plugin_register) ( void );
    gpointer (* plugin_run) ( void );
    GModule * handle;
}  gsb_plugin;

#endif
