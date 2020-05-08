#ifndef _COMPTES_TRAITEMENTS_H
#define _COMPTES_TRAITEMENTS_H (1)
/* START_INCLUDE_H */
#include "gsb_real.h"
#include "gsb_data_account.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkWidget *	gsb_account_create_combo_list 			(GCallback func,
													 gpointer data,
													 gboolean include_closed);
GtkWidget *	gsb_account_create_menu_list 			(GCallback func,
													 gboolean activate_currrent,
													 gboolean include_closed);
gboolean 	gsb_account_delete 						(void);
gint 		gsb_account_get_account_from_combo		(void);
gint 		gsb_account_get_combo_account_number 	(GtkWidget *combo_box);
gboolean 	gsb_account_new 						(KindAccount account_type,
												     gint currency_number,
												     gint bank_number,
												     GsbReal init_amount,
												     const gchar *name,
												     gchar *name_icon);
gboolean 	gsb_account_set_combo_account_number	(GtkWidget *combo_box,
													 gint account_number);
gboolean 	gsb_account_update_combo_list 			(GtkWidget *combo_box,
													 gboolean include_closed);
/* END_DECLARATION */
#endif
