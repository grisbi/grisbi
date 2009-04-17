#ifndef _BANQUE_H
#define _BANQUE_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
GtkWidget *gsb_bank_create_combobox ( gint index,
                        GCallback hook,
                        gpointer data,
                        GCallback default_func,
                        gint number_for_func );
GtkWidget *gsb_bank_create_page ( gboolean default_sensitive );
gboolean gsb_bank_edit_from_button ( GtkWidget *button,
                        GtkWidget *combobox );
gint gsb_bank_list_get_bank_number ( GtkWidget *combobox );
gboolean gsb_bank_list_set_bank ( GtkWidget *combobox,
                        gint bank_number,
                        gint number_for_func );
/* END_DECLARATION */
#endif
