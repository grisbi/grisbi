#ifndef _ETATS_ONGLET_H
#define _ETATS_ONGLET_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "gsb_data_report.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean 	etats_onglet_ajoute_etat 								(void);
GtkWidget *	etats_onglet_create_reports_tab 						(void);
void		etats_onglet_create_search_report						(void);
void 		etats_onglet_create_search_tiers_categ_ib_report		(GtkWindow *dialog,
																	 const gchar *text,
																	 SearchDataReport *search_data_report);
void		etats_onglet_efface_etat 								(void);
void		etats_onglet_force_efface_etat							(void);
GtkWidget *	etats_onglet_get_notebook_etats 						(void);
void 		etats_onglet_reports_toolbar_set_style 					(gint toolbar_style);
void 		etats_onglet_unsensitive_reports_widgets				(void);
void 		etats_onglet_update_gui_to_report 						(gint report_number);
/* END_DECLARATION */
#endif
