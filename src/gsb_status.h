#ifndef _GSB_STATUS_H
#define _GSB_STATUS_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
void gsb_status_clear (  );
GtkWidget *gsb_status_get_status_bar ( void );
void gsb_status_message ( gchar * message );
void gsb_status_stop_wait ( gboolean force_update );
void gsb_status_wait ( gboolean force_update );
void gsb_status_set_status_bar (GtkWidget *status_bar);
/* END_DECLARATION */
#endif
