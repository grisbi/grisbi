#ifndef _OPERATIONS_FORMULAIRE_H
#define _OPERATIONS_FORMULAIRE_H (1)


/* START_INCLUDE_H */
#include "gsb_form_transaction.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void affiche_cache_le_formulaire ( void );
void click_sur_bouton_voir_change ( void );
gboolean gsb_form_finish_edition ( void );
gboolean gsb_form_transaction_complete_form_by_payee ( const gchar *payee_name );
void gsb_form_transaction_fill_form ( gint element_number,
				      gint account_number,
				      gint transaction_number );
gint gsb_form_validate_transfer ( gint transaction_number,
				  gint new_transaction,
				  gchar *name_transfer_account );
gboolean gsb_transactions_list_append_new_transaction ( gint transaction_number );
gboolean gsb_transactions_list_update_transaction ( gint transaction_number );
void place_type_formulaire ( gint no_type,
			     gint no_option_menu,
			     gchar *contenu );
/* END_DECLARATION */
#endif
