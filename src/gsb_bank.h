#ifndef _BANQUE_H
#define _BANQUE_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gint 		gsb_bank_combo_list_get_bank_number		(GtkWidget *combobox);
gboolean 	gsb_bank_combo_list_set_bank 			(GtkWidget *combobox,
													 gint bank_number);
GtkWidget *	gsb_bank_create_combobox 				(gint index);
gboolean 	gsb_bank_edit_from_button 				(GtkWidget *button,
													 GtkWidget *combobox);
void		gsb_bank_free_combo_list_model			(void);
void		gsb_bank_new_combobox_from_ui			(GtkWidget *combo,
													 gint index);
gboolean 	gsb_bank_update_combo_list_model		(GtkWidget *combobox);
/* END_DECLARATION */
#endif
