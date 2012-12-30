#ifndef GSB_PLUGINS_H
#define GSB_PLUGINS_H

#include <glib.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


typedef struct gsb_plugin {
    gchar * name;
    void (* plugin_register) ( void );
    gpointer (* plugin_run) ( );
}  gsb_plugin;


/* START_DECLARATION */
gsb_plugin * gsb_plugin_find ( gchar * plugin_name );
gchar * gsb_plugin_get_list ();
void gsb_plugins_scan_dir ( void );
/* END_DECLARATION */

#endif
