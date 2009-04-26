#ifndef _OPERATIONS_FORMULAIRE_H
#define _OPERATIONS_FORMULAIRE_H (1)


/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean gsb_form_transaction_change_clicked ( GtkWidget *button,
					       gpointer null );
void gsb_form_transaction_check_change_button ( gint currency_number,
						gint account_number );
gboolean gsb_form_transaction_complete_form_by_payee ( const gchar *payee_name );
GSList *gsb_form_transaction_get_parties_list_from_report ( void );
gboolean gsb_form_transaction_recover_splits_of_transaction ( gint new_transaction_number,
							      gint no_last_split );
gint gsb_form_transaction_validate_transfer ( gint transaction_number,
					      gint new_transaction,
					      gint account_transfer );
gint gsb_form_transactions_look_for_last_party ( gint no_party,
						 gint no_new_transaction,
						 gint account_number );
/* END_DECLARATION */
#endif
