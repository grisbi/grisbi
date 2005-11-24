#ifndef _GSB_STATUS_H
#define _GSB_STATUS_H (1)
/* START_DECLARATION */
GtkWidget * gsb_new_statusbar ();
void gsb_status_clear (  );
void gsb_status_message ( gchar * message );
void gsb_status_start_activity (  );
void gsb_status_stop_activity (  );
/* END_DECLARATION */
#endif
