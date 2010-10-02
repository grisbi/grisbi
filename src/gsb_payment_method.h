#ifndef _GSB_PAYMENT_METHOD_H
#define _GSB_PAYMENT_METHOD_H (1)

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean gsb_payment_method_changed_callback ( GtkWidget *combo_box,
                        gpointer null );
gboolean gsb_payment_method_create_combo_list ( GtkWidget *combo_box,
                        gint sign,
                        gint account_number,
                        gint exclude,
                        gboolean contra );
gint gsb_payment_method_get_combo_sign ( GtkWidget *combo_box );
gint gsb_payment_method_get_selected_number ( GtkWidget *combo_box );
gboolean gsb_payment_method_set_combobox_history ( GtkWidget *combo_box,
                        gint payment_number,
                        gboolean check_entry );
void gsb_payment_method_set_payment_position ( GtkWidget *combo_box,
                        gint payment_number );
gboolean gsb_payment_method_show_cheque_entry_if_necessary ( gint payment_number );
/* END_DECLARATION */
#endif
