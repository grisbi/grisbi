#ifndef GSB_PLUGINS_H
#define GSB_PLUGINS_H

#include <glib.h>
#ifndef ENABLE_STATIC
#include <gmodule.h>
#endif

/* START_INCLUDE_H */
/* END_INCLUDE_H */


typedef struct gsb_plugin {
    gchar * name;
    void (* plugin_register) ( void );
    gpointer (* plugin_run) ( );
#ifndef ENABLE_STATIC
    GModule * handle;
#endif
}  gsb_plugin;


/* START_DECLARATION */
gsb_plugin * gsb_plugin_find ( gchar * plugin_name );
gchar * gsb_plugin_get_list ();
void gsb_plugins_scan_dir ( const char *dirname );
/* END_DECLARATION */

#endif
