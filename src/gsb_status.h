#ifndef _GSB_STATUS_H
#define _GSB_STATUS_H (1)

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
GtkWidget * gsb_new_statusbar ();
void gsb_status_clear (  );
void gsb_status_message ( gchar * message );
void gsb_status_stop_wait ( gboolean force_update );
void gsb_status_wait ( gboolean force_update );
/* END_DECLARATION */
#endif
