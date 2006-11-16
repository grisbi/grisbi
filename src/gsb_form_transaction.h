#ifndef _OPERATIONS_FORMULAIRE_H
#define _OPERATIONS_FORMULAIRE_H (1)


/* START_INCLUDE_H */
#include "gsb_form_transaction.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void affiche_cache_le_formulaire ( void );
void click_sur_bouton_voir_change ( void );
GSList *gsb_form_get_parties_list_from_report ( void );
void gsb_form_transaction_check_change_button ( gint currency_number,
						gint account_number );
gboolean gsb_form_transaction_complete_form_by_payee ( const gchar *payee_name );
gint gsb_form_transactions_look_for_last_party ( gint no_party,
						 gint no_new_transaction,
						 gint account_number );
gint gsb_form_validate_transfer ( gint transaction_number,
				  gint new_transaction,
				  gint account_transfer );
gboolean gsb_transactions_list_append_new_transaction ( gint transaction_number );
gboolean gsb_transactions_list_recover_breakdowns_of_transaction ( gint new_transaction_number,
								   gint no_last_breakdown,
								   gint no_account );
gboolean gsb_transactions_list_update_transaction ( gint transaction_number );
/* END_DECLARATION */
#endif
