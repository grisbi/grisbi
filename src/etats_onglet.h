#ifndef _ETATS_ONGLET_H
#define _ETATS_ONGLET_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean ajout_etat ( void );
GtkWidget *creation_onglet_etats ( void );
void efface_etat ( void );
GtkWidget *etats_onglet_get_notebook_etats ( void );
void gsb_gui_unsensitive_report_widgets ();
void gsb_gui_update_gui_to_report ( gint report_number );
void gsb_gui_update_reports_toolbar ( void );
/* END_DECLARATION */
#endif
